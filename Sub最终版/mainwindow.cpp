#define low_it 6000
#define med_it 12000
#define high_it 18000
#define IP "192.168.1.115"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //绑定选择项
    open_BG = new QButtonGroup;
    open_BG->addButton(ui->open_radioButton,0);
    open_BG->addButton(ui->close_radioButton,1);
    ui->close_radioButton->setChecked(true);
    speed_BG = new QButtonGroup;
    speed_BG->addButton(ui->high_radioButton,0);
    speed_BG->addButton(ui->med_radioButton,1);
    speed_BG->addButton(ui->low_radioButton,2);
    //初始化界面
    init_ctemp = 18;
    current_cost = 0;
    room_id = 0;
    ui->status_label->setText("关机");
    ui->ctemp_label->setText(QString::number(init_ctemp));
    ui->cost_label->setText("---");
    ui->blow_label->setText("---");
    ui->mode_label->setText("---");
    ui->dtemp_lable->setText("---");
    ui->time_label->setText("---");

    //建立网络连接
    tcpSocket = new QTcpSocket(this);
    connect(tcpSocket,SIGNAL(readyRead()),this,SLOT(readMessage()));

    timer = new QTimer();
    timer->setInterval(low_it);
    //启动定时器
    timer->start();
    connect(speed_BG,SIGNAL(buttonClicked(int)),this,SLOT(timeIntervalChange(int)));
    connect(timer, SIGNAL(timeout()), this, SLOT(onTimeOut()));



    ack_timer = new QTimer();
    ack_timer->setInterval(1000);
    connect(ack_timer, SIGNAL(timeout()), this, SLOT(ack_onTimeOut()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

//发送SYN
void MainWindow::newConnect()
{
    tcpSocket->abort();//取消已有的连接
    //连接到主机，这里从界面获取主机地址和端口号
    tcpSocket->connectToHost(QHostAddress(IP),8080);
    QString ctemp = ui->ctemp_label->text();

    //用于暂存我们要发送的数据
    QString message = "{\"From\":"+ QString::number(room_id)
            +",\"Type\":\"SYN\",\"Action\":{\"Ctemp\":"+ctemp
            +",\"Dtemp\":" + QString("%1").arg(0)
            +",\"speed\":\"" + "High"
            +"\",\"workmode\":\""+"Cooling"+"\"}}\n"; ;

    std::string str = message.toStdString();
    const char* ch = str.c_str();
    tcpSocket->write(ch);
}

//发送req
void MainWindow::sendMessage()
{
    tcpSocket->abort();//取消已有的连接
    //连接到主机，这里从界面获取主机地址和端口号
    tcpSocket->connectToHost(QHostAddress(IP),8080);

    float ctemp = ui->ctemp_label->text().toFloat();
    float dtemp = ui->dtemp_lable->text().toFloat();
    int sp = this->speed_BG->checkedId();
    QString speed;
    if(sp == 0)
        speed = "High";
    else if(sp == 1)
        speed = "Medium";
    else
        speed = "Low";
    QString workmode = ui->mode_label->text();

    //用于暂存我们要发送的数据
    QString message = "{\"From\":"+ QString::number(room_id)
            +",\"Type\":\"req\",\"Action\":{\"Ctemp\":"+ QString("%1").arg(ctemp)
            +",\"Dtemp\":" + QString("%1").arg(dtemp)
            +",\"speed\":\"" + speed
            +"\",\"workmode\":\""+workmode+"\"}}\n";

    qDebug()<<"发送了："<<message;

    std::string str = message.toStdString();
    const char* ch = str.c_str();
    tcpSocket->write(ch);

    ack_timer->start();
}

//接收ack
void MainWindow::readMessage()
{
    QTextStream in(tcpSocket);
    //存放从服务器接收到的字符串
    QString message;
    in>>message;
    qDebug()<<"收到的消息是："<<message;

    qDebug()<<"成功进入";
    QJsonParseError error;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(message.toUtf8(), &error);
    if (error.error == QJsonParseError::NoError) {
        if (jsonDocument.isObject()) {
            QVariantMap result = jsonDocument.toVariant().toMap();

            if(result["From"].toInt() != room_id)
                return;

            if(result.contains("Cost"))
            {
                float cost = result["Cost"].toFloat();
               // float ccost = ui->cost_label->text().toFloat();
                ui->cost_label->setText(QString("%1").arg(cost));
                int sp = this->speed_BG->checkedId();
                if(sp == 0)
                    ui->blow_label->setText("High");
                else if(sp == 1)
                    ui->blow_label->setText("Medium");
                else
                    ui->blow_label->setText("Low");

                ui->status_label->setText("服务中");
                ack_timer->stop();
            }
            else
            {
                QVariantMap nestedMap = result["Action"].toMap();
                int dTemp = nestedMap["Dtemp"].toFloat();
                QString speed = nestedMap["speed"].toString();
                QString workmode = nestedMap["workmode"].toString();

                ui->cost_label->setText(QString::number(current_cost));
                ui->dtemp_lable->setText(QString("%1").arg(dTemp));
                ui->blow_label->setText(speed);
                ui->mode_label->setText(workmode);
                ui->status_label->setText("待机");
                timer->setInterval(low_it);
                timer->start();
                if(speed == "High")
                    ui->high_radioButton->setChecked(true);
                else if(speed == "Medium")
                    ui->med_radioButton->setChecked(true);
                else
                    ui->low_radioButton->setChecked(true);

                sendMessage();
            }
        }
    }
    else
    {
        qFatal(error.errorString().toUtf8().constData());
        exit(1);
    }
}

void MainWindow::timeIntervalChange(int id)
{
    if(id == 2)//低
        timer->setInterval(high_it);//设置定时器每隔30s发送一个timeout()信号
    else if(id == 1)
        timer->setInterval(med_it);
    else
        timer->setInterval(low_it);
    timer->start();
}

void MainWindow::onTimeOut()
{
    float ctemp = ui->ctemp_label->text().toFloat();
    float dtemp = ui->dtemp_lable->text().toFloat();
    QString speed = ui->blow_label->text();
    QString workmode = ui->mode_label->text();
    QString status = ui->status_label->text();

    int data;
    if(speed == "High")
        data = 3;
    else if(speed == "Medium")
        data = 2;
    else if(speed == "Low")
        data = 1;
    else
        data = 0;

    if(status == "服务中")
    {
        if(ctemp > dtemp)
            ctemp = qMax(ctemp-1,dtemp);
        else if(ctemp < dtemp)
            ctemp = qMin(ctemp+1,dtemp);
        else
        {
            status = "待机";
            timer->setInterval(low_it);
            timer->start();
        }
    }
    else
    {
        if(workmode == "Cooling" && ctemp < init_ctemp)
            ctemp += 0.5;
        else if(workmode == "Heating" && ctemp > init_ctemp)
            ctemp -= 0.5;
    }
    ui->ctemp_label->setText(QString("%1").arg(ctemp));
    ui->status_label->setText(status);
    if(ctemp == (int)ctemp && ui->status_label->text() != "关机")
        sendMessage();
}

void MainWindow::ack_onTimeOut()
{
    ui->status_label->setText("待机");
    timer->setInterval(low_it);
    timer->start();
    ack_timer->stop();
}

void MainWindow::on_open_radioButton_clicked()
{
    newConnect();
}

bool MainWindow::check_temp(int temp)
{
    if(ui->mode_label->text() == "Cooling")
        return (temp >= 18 && temp <= 25);
    return (temp >= 25 && temp <= 30);
}

void MainWindow::on_add_pbt_clicked()
{
    int ld = ui->dtemp_lable->text().toFloat();
    if(check_temp(ld + 1))
    {
        ui->dtemp_lable->setText(QString("%1").arg(ld + 1));
        sendMessage();
    }
}


void MainWindow::on_dec_pbt_clicked()
{
    int ld = ui->dtemp_lable->text().toFloat();
    if(check_temp(ld - 1))
    {
        ui->dtemp_lable->setText(QString("%1").arg(ld - 1));
        sendMessage();
    }
}

void MainWindow::on_high_radioButton_clicked()
{
    sendMessage();
}

void MainWindow::on_med_radioButton_clicked()
{
    sendMessage();
}

void MainWindow::on_low_radioButton_clicked()
{
    sendMessage();
}

void MainWindow::on_close_radioButton_clicked()
{
    ui->status_label->setText("关机");
    ack_timer->stop();

    tcpSocket->abort();//取消已有的连接
    //连接到主机，这里从界面获取主机地址和端口号
    tcpSocket->connectToHost(QHostAddress(IP),8080);

    //用于暂存我们要发送的数据
    QString message = "{\"From\":"+ QString::number(room_id) + ",\"Type\":\"DEL\"}\n";
    qDebug()<<"发送了："<<message;
    std::string str = message.toStdString();
    const char* ch = str.c_str();
    tcpSocket->write(ch);
}

void MainWindow::on_pushButton_clicked()
{
    room_id = ui->comboBox->currentIndex();
}
