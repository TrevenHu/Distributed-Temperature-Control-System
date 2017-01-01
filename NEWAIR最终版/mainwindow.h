#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QTcpServer>
#include <QTcpSocket>
#include <QtNetwork>
#include <QMainWindow>
#include <QButtonGroup>
#include <QTextStream>
#include <QSqlTableModel>
#include <statusinfo.h>
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void sendMessage(int type,QString content,int room_id);
    void dispatch(); //调度函数
    void refresh(); //刷新从控机状态显示

private slots:

    void readMessage();

    void dataProcess();

    void on_open_btn_clicked();

    void on_ldetail_rbtn_clicked();

    void on_toolButton_clicked();

    void on_toolButton_5_clicked();

    void on_toolButton_3_clicked();

    void on_ldetail_rbtn_2_clicked();

    void on_month_rbtn_clicked();

    void on_day_rbtn_clicked();

    void on_week_rbtn_clicked();

private:
    Ui::MainWindow *ui;
    QButtonGroup *buttonGroup;
    QSqlTableModel *model;

    StatusInfo roomStatus[4];
    bool is_open[4];

    QTcpServer *tcpServer;
    QTcpSocket *tcpSocket;
    QString message;  //存放从服务器接收到的字符串
};

#endif // MAINWINDOW_H
