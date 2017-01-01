#include "mytcp.h"

myTCP::myTCP()
{
    tcpServer = new QTcpServer();
    if(!tcpServer->listen(QHostAddress::LocalHost,6666))
    {  //**本地主机的6666端口，如果出错就输出错误信息，并关闭
        qDebug() << tcpServer->errorString();
        tcpServer->close();
    }
    //连接信号和相应槽函数
    connect(tcpServer,&QTcpServer::newConnection,this,&myTCP::sendMessage);
}

void myTCP::sendMessage()
{
    QByteArray block; //用于暂存我们要发送的数据
    QDataStream out(&block,QIODevice::WriteOnly);  //使用数据流写入数据
    out.setVersion(QDataStream::Qt_5_2);  //设置数据流的版本，客户端与服务器端使用的版本要相同
    out<<(quint16)0;  //添加一个quint16大小的空间，用于后面存在文件的大小信息
//    out<<tr("Hello tcp!");
    out.device()->seek(0);   //指定当前读写位置
    out<<(quint16)(block.size()-sizeof(quint16));  //实际文件的大小信息

    QTcpSocket *clientConnect = tcpServer->nextPendingConnection(); //获取已建立连接的子套接字
    connect(clientConnect,&QTcpSocket::disconnected,clientConnect,&QTcpSocket::deleteLater);
    clientConnect->write(block);
    clientConnect->disconnectFromHost();  //当发送完成时就会断开连接
 //   ui->statusLabel->setText("sent message successful!!!");
}

