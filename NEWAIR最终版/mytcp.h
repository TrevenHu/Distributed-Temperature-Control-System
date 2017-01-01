#ifndef MYTCP_H
#define MYTCP_H
#include <QtNetWork>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>
#include <QByteArray>
#include <QDataStream>
class myTCP
{
public:
    myTCP();

private:
    QTcpServer *tcpServer;

private slots:
    void sendMessage();
};

#endif // MYTCP_H
