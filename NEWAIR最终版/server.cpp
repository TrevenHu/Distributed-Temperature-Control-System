#include "server.h"
#include "serversocket.h"
#include <QDebug>
#include <QSqlRecord>
Server::Server(QObject *parent) :
    QTcpServer(parent)
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("myDB.db");
    if(!db.open())
    {
        qDebug()<< "打开数据库出错";
    }
}

bool Server::login(QString userName, QString passWord)
{
    QSqlQuery query_select(QString("SELECT * FROM userInfo where userName=\"%1\" and passWord=\"%2\" ").arg(userName).arg(passWord));

    while(query_select.next())
    {
        if(query_select.value("userName").toString()=="" )
        {
            return false;
        }else
        {
            return true;
        }
    }
    return false;
}

QString Server::getInfoJson(QString userName)
{
    qDebug()<<userName;
    QString sex;
    QString age;
    QString height;
    QSqlQuery query_select(QString("SELECT * FROM userInfo where userName=\"%1\"").arg(userName));
    while (query_select.next())
    {
        sex =query_select.value("sex").toString();
        age = query_select.value("age").toString();
        height = query_select.value("height").toString();
    }

    qDebug()<<sex;
    QString a=QString("{\"type\":\"getInfo result\",\"sex\":\"%1\",\"age\":\"%2\", \"height\":\"%3\"}").arg(sex).arg(age).arg(height);
    return a;
}
void Server::incomingConnection(int serverSocketID)
{
    ServerSocket *serverSocket=new ServerSocket(this,serverSocketID);
    serverSocket->setSocketDescriptor(serverSocketID);

    connect(serverSocket,SIGNAL(serverSocketReadData(int,QString,int,QByteArray)),this,SLOT(ReadData(int,QString,int,QByteArray)));
    connect(serverSocket,SIGNAL(serverSocketDisConnect(int,QString,int)),this,SLOT(DisConnect(int,QString,int)));
    connect(serverSocket,SIGNAL(aboutToClose()),this,SLOT(deleteLater()));//关闭监听时，对象自动删除



    idSocketMap.insert(serverSocketID,serverSocket);
    serverSocketCount++;

     emit hasConnect(serverSocketID, serverSocket->peerAddress().toString(),serverSocket->peerPort());

    qDebug()<<serverSocketCount;
}

void Server::ReadData(int serverSocketID,QString IP,int Port,QByteArray data)
{
    emit hasData(serverSocketID,IP,Port,data);
    QJsonDocument jsonAnalyse;
    QJsonParseError *jsonError=new QJsonParseError;
    jsonAnalyse=QJsonDocument::fromJson(data,jsonError);
    if(! jsonError->error==QJsonParseError::NoError)
    {
        qDebug()<< "接收到的数据不完整或出错";
        return;
    }
    if(jsonAnalyse.isObject())
    {
        QJsonObject obj=jsonAnalyse.object();//取得最外层这个大对象
        QString str=obj["type"].toString();
        QByteArray arrSend;
        if(str=="login")
        {
            QString userName = obj["userName"].toString();
            QString passWord = obj["passWord"].toString();
            if(login(userName,passWord))
            {
                qDebug()<< userName << "登陆成功";
                idUserMap.insert(serverSocketID,userName);
                arrSend.append("{\"type\":\"login result\",\"result\":\"success\"}");
            }else
            {
                qDebug()<< userName << "登陆失败";
                arrSend.append("{\"type\":\"login result\",\"result\":\"failed\"}");
            }
        }else if(str=="getInfo")
        {
            if(idUserMap.contains(serverSocketID))
            {
                QString a=getInfoJson(idUserMap.value(serverSocketID));
                arrSend=a.toLatin1();
            }else
            {
                arrSend.append("{\"type\":\"login result\",\"result\":\"do not logined\"}");

            }
        }
        ServerSocket *Socket = idSocketMap.value(serverSocketID);
        Socket->write(arrSend);
    }



}

void Server::DisConnect(int serverSocketID,QString IP,int Port)
{
    idSocketMap.remove(serverSocketID);
    idUserMap.remove(serverSocketID);
    serverSocketCount--;
    emit hasDisConnect(serverSocketID,IP,Port);
}
