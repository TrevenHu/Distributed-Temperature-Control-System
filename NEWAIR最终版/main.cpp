#include "mainwindow.h"
#include "connection.h"
#include "statusinfo.h"
#include <QApplication>
#include <QJsonDocument>
#include <QJsonObject>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //设置全局文字大小
    QFont font  = a.font();
    font.setPointSize(11);
    a.setFont(font);
    if(!createConnection())
    {
        return 1;
    }
    MainWindow w;
    w.show();

    return a.exec();
}
