#include "serversocket.h"
#include <QDateTime>
#include <QCoreApplication>
ServerSocket::ServerSocket(QObject *parent,int serverSocketID) :
    QTcpSocket(parent)
{
    this->serverSocketID=serverSocketID;
    connect(this,SIGNAL(readyRead()),this,SLOT(ReadData()));//挂接读取数据信号
    connect(this,SIGNAL(disconnected()),this,SLOT(DisConnect()));//关闭连接时，发送断开连接信号
    connect(this,SIGNAL(disconnected()),this,SLOT(deleteLater()));//关闭连接时，对象自动删除

    connect(this,SIGNAL(error(QAbstractSocket::SocketError)),
            this,SLOT(outPutError(QAbstractSocket::SocketError)));

}
void ServerSocket::ReadData()
{

    QTime dieTime = QTime::currentTime().addMSecs(100);
    while( QTime::currentTime() < dieTime )
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

    //读取完整一条数据并发送信号
    QByteArray data=this->readAll();
    emit serverSocketReadData(this->serverSocketID,this->peerAddress().toString(),this->peerPort(),data);
}

void ServerSocket::DisConnect()
{
    //断开连接时，发送断开信号
    emit serverSocketDisConnect(this->serverSocketID,this->peerAddress().toString(),this->peerPort());
}

void ServerSocket::outPutError(QAbstractSocket::SocketError)
{
    this->disconnectFromHost();
    qDebug()<< this->errorString();
}
