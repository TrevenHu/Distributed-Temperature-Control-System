#ifndef SERVER_H
#define SERVER_H
#include "serversocket.h"
#include <QTcpServer>
#include <QMap>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QSqlDatabase>
#include <QSqlQuery>

class Server : public QTcpServer
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = 0);
    bool login(QString userName,QString passWord);//通过查询sqlite数据库对比用户名进行登陆，登陆后，将登陆成功的用户添加到用户列表QMap<int,QString> idUserMap;
    QString getInfoJson(QString userName);
    int serverSocketCount;//用于统计连接的用户数，
private:
    QSqlDatabase db;

    QMap<int,ServerSocket *> idSocketMap;
    QMap<int,QString> idUserMap;



protected:
    void incomingConnection(int serverSocketID);

signals:
    void hasData(int serverSocketID,QString IP,int Port,QByteArray data);//这三个信号用于告诉dialog信息，用于dialog的UI的信息显示，比如更新显示用户连接数
    void hasConnect(int serverSocketID,QString IP,int Port);
    void hasDisConnect(int serverSocketID,QString IP,int Port);

private slots:
    void ReadData(int serverSocketID,QString IP,int Port,QByteArray data);//连接到ServerSocket的void serverSocketReadData……信号
    void DisConnect(int serverSocketID,QString IP,int Port);


};

#endif // SERVER_H
