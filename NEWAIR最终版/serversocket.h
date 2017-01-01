#ifndef SERVERSOCKET_H
#define SERVERSOCKET_H

#include <QTcpSocket>
#include <QHostAddress>
#include <QAbstractSocket>

class ServerSocket : public QTcpSocket
{
    Q_OBJECT
public:
    explicit ServerSocket(QObject *parent = 0,int serverSocketID=0);//在这里为它多加了serverSocketID，将TcpServer的SocketDescriptor传进来。
signals:
    void serverSocketReadData(int serverSocketID,QString IP,int Port,QByteArray data);//自建的server类，通过这个信号，得到传输来的数据的来源及具体内容
    void serverSocketDisConnect(int serverSocketID,QString IP,int Port);//断开连接时，通过这个信号，S<span style="font-family: Arial, Helvetica, sans-serif;">erver类将容器idSocketMap内对应的套接字删除</span>
private:
    int serverSocketID;
private slots:
    void ReadData();//具体实现读取数据
    void DisConnect();
    void outPutError(QAbstractSocket::SocketError);//qDebug输出套接字出错信息


};

#endif // SERVERSOCKET_H
