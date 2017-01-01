#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QButtonGroup>
#include <QMainWindow>
#include <QTcpSocket>
#include <QtNetwork>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QVariantMap>
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void initial();
    bool check_temp(int temp);

public slots:
    void onTimeOut();
    void ack_onTimeOut();
    void timeIntervalChange(int id);

private:
    Ui::MainWindow *ui;
    QButtonGroup *open_BG,*speed_BG;
    QTcpSocket *tcpSocket;
    QTimer *timer,*ack_timer;
    int init_ctemp;
    int current_cost;
    int room_id;

private slots:
    void newConnect(); //发送SYN
    void readMessage();//接收ack
    void sendMessage();//发送req
    void on_open_radioButton_clicked();
    void on_add_pbt_clicked();
    void on_dec_pbt_clicked();
    void on_high_radioButton_clicked();
    void on_med_radioButton_clicked();
    void on_low_radioButton_clicked();
    void on_close_radioButton_clicked();
    void on_pushButton_clicked();
};

#endif // MAINWINDOW_H
