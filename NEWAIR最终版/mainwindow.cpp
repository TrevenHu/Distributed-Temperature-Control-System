#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include "statusinfo.h"
#include "connection.h"
#define IP "192.168.1.115"
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    buttonGroup = new QButtonGroup;
    buttonGroup->addButton(ui->cold_radioButton,0);
    buttonGroup->addButton(ui->hot_radioButton,1);
    ui->cold_radioButton->setChecked(true);

    memset(is_open,0,sizeof(is_open));
    refresh();

    tcpServer = new QTcpServer();
    if(!tcpServer->listen(QHostAddress(IP),8080))
    {  //**本地主机的8080端口，如果出错就输出错误信息，并关闭
        qDebug() << tcpServer->errorString();
    }
    //连接信号和相应槽函数
    connect(tcpServer,&QTcpServer::newConnection,this,&MainWindow::readMessage);
}

MainWindow::~MainWindow()
{
    tcpServer->close();
    delete ui;
}

void MainWindow::readMessage()
{
    tcpSocket = tcpServer->nextPendingConnection(); //获取已建立连接的子套接字
    connect(tcpSocket,&QTcpSocket::readyRead,this,&MainWindow::dataProcess);
}

void MainWindow::dataProcess()
{
    if(tcpSocket->bytesAvailable()>0)
    {
        QTextStream in(tcpSocket);
        //存放从服务器接收到的字符串
        in>>message;
       // qDebug()<<"收到了："<<message;

        //接收消息
        StatusInfo newInfo(message);
        int room_id = newInfo.from;
        StatusInfo &lastInfo = roomStatus[room_id];

        if(newInfo.type == "SYN")
        {
            float temp = newInfo.cTemp;
            lastInfo.cTemp = temp;
            newInfo = roomStatus[room_id];
            QString content = "{\"From\":"+ QString::number(room_id)
                                +",\"Type\":\"ack\",\"Action\":{\"Ctemp\":"+QString("%1").arg(temp)
                                +",\"Dtemp\":" + QString("%1").arg(newInfo.dTemp)
                                +",\"speed\":\"" + newInfo.speed
                                +"\",\"workmode\":\""+newInfo.workmode+"\"}}\n";
            sendMessage(0,content,room_id);

            is_open[room_id] = true;
            roomStatus[room_id].open_times++;
            qDebug()<<"开关次数："<<roomStatus[room_id].open_times;
            return;
        }

        if(newInfo.type == "DEL")
        {
            lastInfo.status = 0;
            is_open[room_id] = false;
            refresh();
            //更新消息队列后，调用调度函数
            dispatch();
            return;
        }

        if(lastInfo.status == 0|| newInfo.is_changed(lastInfo))
        {
            if(newInfo.get_req(lastInfo) == 1)
            {
              //  qDebug()<<"风速设置更改";
                newInfo.speed_times++;
            }
            else
            {
             //   qDebug()<<"温控设置更改";
                newInfo.temp_times++;
            }
        }

       // qDebug()<<newInfo.cost<<lastInfo.cost<<newInfo.cTemp <<lastInfo.cTemp<<qAbs(newInfo.cTemp - lastInfo.cTemp);
       // qDebug()<<lastInfo.status;
        //若之前该房间无请求，则直接覆盖
        if(lastInfo.status == 0)
        {
            newInfo.open_times += lastInfo.open_times;
            newInfo.temp_times += lastInfo.temp_times;
            newInfo.speed_times += lastInfo.speed_times;
            newInfo.dispatch_times += lastInfo.dispatch_times;
            lastInfo = newInfo;
        }
        else if(newInfo.is_changed(lastInfo))
        {
            //如果温差是由于空调服务引起的，叠加费用
            if(lastInfo.status == 2)
                newInfo.cost = lastInfo.cost + qAbs(newInfo.cTemp - lastInfo.cTemp);
            newInfo.serveTime += lastInfo.serveTime;
            newInfo.bTemp = lastInfo.bTemp;
            newInfo.open_times += lastInfo.open_times;
            newInfo.temp_times += lastInfo.temp_times;
            newInfo.speed_times += lastInfo.speed_times;
            newInfo.dispatch_times += lastInfo.dispatch_times;
            lastInfo = newInfo;
        }
        else//当前消息是室温变化
        {   //如果温差是由于空调服务引起的，叠加费用
            if(lastInfo.status == 2)
                lastInfo.cost += qAbs(newInfo.cTemp - lastInfo.cTemp);
            lastInfo.cTemp = newInfo.cTemp;
            lastInfo.open_times += newInfo.open_times;
        }
        refresh();
        //更新消息队列后，调用调度函数
        dispatch();

        if(lastInfo.status == 2)
            sendMessage(1,QString("%1").arg(lastInfo.cost),room_id);
    }
}

//调度函数
void MainWindow::dispatch()
{
   // qDebug()<<"调用了dispatch()";
    int cnt = 0;//统计当前请求个数
    for(int i = 0; i < 4; i++)
    {
        if(is_open[i] == false) continue;
        StatusInfo &Info = roomStatus[i];
        //若室温等于目标温度，将该请求设置为“已完成”
        if(Info.cTemp == Info.dTemp)
        {
            Info.status = 0;
            insert_ldetail(Info);
        }
        if(Info.status != 0)
            cnt++;
    }
   // qDebug()<<"cnt = "<<cnt;
    if(cnt!=4)
    {
        for(int i = 0; i < 4; i++)
        {
            StatusInfo &Info = roomStatus[i];
            if(Info.status == 0)
                continue;
            else
            {
                if(Info.status != 2)
                {
                    Info.dispatch_times++;
                    Info.status = 2;
                }
            }
        }
        refresh();
        return;
    }
    //找到优先级最低的请求
    int lowest_id = 0;
    for(int i = 1; i < 4; i++)
    {
        StatusInfo &Info = roomStatus[i];
        if(roomStatus[lowest_id].is_high(Info))
            lowest_id = i;
    }
    //修改当前状态
    StatusInfo &lInfo = roomStatus[lowest_id];
    if(lInfo.status == 2)
    {
        lInfo.status = 1;
        lInfo.dispatch_times++;
        lInfo.serveTime += lInfo.lTime.secsTo(QDateTime::currentDateTime());
    }
    for(int i = 0; i < 4; i++)
    {
        if(i == lowest_id) continue;
        StatusInfo &Info = roomStatus[i];
        if(Info.status == 1)
        {
            Info.status = 2;
            lInfo.dispatch_times++;
            Info.lTime = QDateTime::currentDateTime();
        }
    }
    refresh();
}

//0表示初始化，1表示ack
void MainWindow::sendMessage(int type,QString content,int room_id)
{
    QString info;
    if(type == 0)
        info = content;
    else
        info = "{\"From\":" + QString::number(room_id) +",\"Type\":\"ack\",\"Cost\":\"" +content + "\"}\n";

  //  QTcpSocket *clientConnect = tcpServer->nextPendingConnection(); //获取已建立连接的子套接字
    connect(tcpSocket,&QTcpSocket::disconnected,tcpSocket,&QTcpSocket::deleteLater);

   // qDebug()<<"发送了："<<info;
    std::string str = info.toStdString();
   // connect(tcpSocket,SIGNAL(disconnected()),tcpSocket,
     //          SLOT(deleteLater()));

    const char* ch = str.c_str();
    tcpSocket->write(ch);
   // tcpSocket->disconnectFromHost();  //当发送完成时就会断开连接
}

void MainWindow::on_open_btn_clicked()
{
    StatusInfo Info;
    Info.type = "req";
    Info.cTemp = Info.bTemp = 30;
    Info.dTemp = (int) ui->temp_spinBox->value();
    int temp = (int) ui->speed_spinBox->value();

    if(temp == 1)    Info.speed = "Low";
    else if(temp == 2)    Info.speed = "Medium";
    else if(temp ==3)    Info.speed = "High";
    else     Info.speed = "NULL";

    temp = this->buttonGroup->checkedId();
    if(temp == 0)   Info.workmode = "Cooling";
    else    Info.workmode = "Heating";
    Info.bTime = QDateTime::currentDateTime();
    Info.status = 1;
    Info.serveTime = 0;
    Info.cost = 0;
    Info.open_times = 0;
    Info.temp_times = 0;
    Info.speed_times = 0;
    Info.dispatch_times = 0;

    for(int i=0; i < 4; i++)
    {
        Info.from = i;
        roomStatus[i] = Info;
    }
    dispatch();

    QMessageBox::information(this,"提示对话框","中央空调开机成功！",QMessageBox::Ok);
}


//刷新从控机状态显示
void MainWindow::refresh()
{
    int temp;
   // qDebug()<<"调用了refresh()";
    if(is_open[0] == true)
    {
        //roomStatus[0].print();
        temp = roomStatus[0].status;
        if(temp == 0) ui->state->setText("No Request");
        else if(temp == 1) ui->state->setText("Wait");
        else ui->state->setText("Serve");
        ui->speed->setText(roomStatus[0].speed);
        ui->workmode->setText(roomStatus[0].workmode);
        ui->cur_temp->setText(QString("%1").arg(roomStatus[0].cTemp));
        ui->temp->setText(QString("%1").arg(roomStatus[0].dTemp));
        ui->cur_cost->setText(QString("%1").arg(roomStatus[0].cost));
    }
    else
    {
        ui->state->setText("关机");
        ui->speed->setText("---");
        ui->workmode->setText("---");
        ui->cur_temp->setText("---");
        ui->temp->setText("---");
        ui->cur_cost->setText("---");
    }

    if(is_open[1] == true)
    {
        temp = roomStatus[1].status;
        if(temp == 0) ui->state_2->setText("No Request");
        else if(temp == 1) ui->state_2->setText("Wait");
        else ui->state_2->setText("Serve");
        ui->speed_2->setText(roomStatus[1].speed);
        ui->workmode_2->setText(roomStatus[1].workmode);
        ui->cur_temp_2->setText(QString("%1").arg(roomStatus[1].cTemp));
        ui->temp_2->setText(QString("%1").arg(roomStatus[1].dTemp));
        ui->cur_cost_2->setText(QString("%1").arg(roomStatus[1].cost));
    }
    else
    {
        ui->state_2->setText("关机");
        ui->speed_2->setText("---");
        ui->workmode_2->setText("---");
        ui->cur_temp_2->setText("---");
        ui->temp_2->setText("---");
        ui->cur_cost_2->setText("---");
    }

    if(is_open[2] == true)
    {
        temp = roomStatus[2].status;
        if(temp == 0) ui->state_3->setText("No Request");
        else if(temp == 1) ui->state_3->setText("Wait");
        else ui->state_3->setText("Serve");
        ui->speed_3->setText(roomStatus[2].speed);
        ui->workmode_3->setText(roomStatus[2].workmode);
        ui->cur_temp_3->setText(QString("%1").arg(roomStatus[2].cTemp));
        ui->temp_3->setText(QString("%1").arg(roomStatus[2].dTemp));
        ui->cur_cost_3->setText(QString("%1").arg(roomStatus[2].cost));
    }
    else
    {
        ui->state_3->setText("关机");
        ui->speed_3->setText("---");
        ui->workmode_3->setText("---");
        ui->cur_temp_3->setText("---");
        ui->temp_3->setText("---");
        ui->cur_cost_3->setText("---");
    }

    if(is_open[3] == true)
    {
        temp = roomStatus[3].status;
        if(temp == 0) ui->state_4->setText("No Request");
        else if(temp == 1) ui->state_4->setText("Wait");
        else ui->state_4->setText("Serve");
        ui->speed_4->setText(roomStatus[3].speed);
        ui->workmode_4->setText(roomStatus[3].workmode);
        ui->cur_temp_4->setText(QString("%1").arg(roomStatus[3].cTemp));
        ui->temp_4->setText(QString("%1").arg(roomStatus[3].dTemp));
        ui->cur_cost_4->setText(QString("%1").arg(roomStatus[3].cost));
    }
    else
    {
        ui->state_4->setText("关机");
        ui->speed_4->setText("---");
        ui->workmode_4->setText("---");
        ui->cur_temp_4->setText("---");
        ui->temp_4->setText("---");
        ui->cur_cost_4->setText("---");
    }
}

//查询账单
void MainWindow::on_toolButton_5_clicked()
{
    ui->tabWidget->setCurrentIndex(0);
    model = new QSqlTableModel(this);
    model->setTable("detail");
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->select();
    ui->cost_tableView->setModel(model);
}
void MainWindow::on_ldetail_rbtn_2_clicked()
{
    int room = (int) ui->detail_spinBox_2->value();
       //根据姓名进行筛选
    model->setFilter(QString("room_id = '%1'").arg(room));
    //显示结果
    model->select();
}

//查询详单
void MainWindow::on_toolButton_clicked()
{
    ui->tabWidget->setCurrentIndex(1);
    model = new QSqlTableModel(this);
    model->setTable("light_detail");
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->select();
    ui->detail_tableView->setModel(model);
}
void MainWindow::on_ldetail_rbtn_clicked()
{
    int room = (int) ui->detail_spinBox->value();
       //根据姓名进行筛选
    model->setFilter(QString("room_id = '%1'").arg(room));
    //显示结果
    model->select();
}

//查询日报表
void MainWindow::on_toolButton_3_clicked()
{
    ui->tabWidget->setCurrentIndex(2);
    model = new QSqlTableModel(this);
    model->setTable("detail");
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->select();
    ui->dd_tableView->setModel(model);
}

void MainWindow::on_day_rbtn_clicked()
{
    QDate day = ui->dateEdit->date();
       //根据姓名进行筛选
    model->setFilter(QString("day = '%1'").arg(day.toString("yyyy-MM-dd")));
    //显示结果
    model->select();
}

void MainWindow::on_week_rbtn_clicked()
{
    QDate day = ui->dateEdit->date();
    QDate end_day = day.addDays(6);
    //根据姓名进行筛选
    model->setFilter(QString("day >= '%1' and day <= '%2'")
                     .arg(day.toString("yyyy-MM-dd"))
                     .arg(end_day.toString("yyyy-MM-dd")));
    //显示结果
    model->select();
}

void MainWindow::on_month_rbtn_clicked()
{
    QDate day = ui->dateEdit->date();
    QDate end_day = day.addDays(29);
    //根据姓名进行筛选
    model->setFilter(QString("day >= '%1' and day <= '%2'")
                     .arg(day.toString("yyyy-MM-dd"))
                     .arg(end_day.toString("yyyy-MM-dd")));
    //显示结果
    model->select();
}
