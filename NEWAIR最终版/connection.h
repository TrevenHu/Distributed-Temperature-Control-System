#ifndef CONNECTION_H
#define CONNECTION_H
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QString>
#include <statusinfo.h>
static bool createConnection()
{
    QSqlDatabase db;
    if(QSqlDatabase::contains("qt_sql_default_connection"))
      db = QSqlDatabase::database("qt_sql_default_connection");
    else
      db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("database.db");
    if(!db.open()) return false;

    QStringList tables=db.tables();
    if(!tables.contains("light_detail",Qt::CaseInsensitive))
    {
        QSqlQuery query;
        query.exec("create table light_detail(room_id int,day Qdate,begin_time Qtime,end_time Qtime,begin_temp float,end_temp float,speed QString,primary key (room_id,day,begin_time))");
        query.exec("create table detail(room_id int,day Qdate,serve_time int,cost float,open_times int,temp_times int,speed_times int,dispatch_times int,primary key(room_id,day))");
    }
    return true;
}

static void insert_ldetail(StatusInfo Info)
{
    QSqlDatabase db;
    if(QSqlDatabase::contains("qt_sql_default_connection"))
      db = QSqlDatabase::database("qt_sql_default_connection");
    else
      db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("database.db");
    db.open();

    QSqlQuery query("select * from light_detail");
    query.last();
    int id = query.at();
    if(id==-2)
        id=-1;
    query.prepare("insert into light_detail values(?,?,?,?,?,?,?)");
    query.addBindValue(Info.from);
    query.addBindValue(Info.bTime.date());
    query.addBindValue(Info.bTime.time());
    query.addBindValue(QTime::currentTime());
    query.addBindValue(Info.bTemp);
    query.addBindValue(Info.cTemp);
    query.addBindValue(Info.speed);
    query.exec();

    int serveTime = Info.bTime.secsTo(QDateTime::currentDateTime());
    qDebug()<<"服务时长："<<serveTime<<"开关次数："<<Info.open_times;

    query.prepare("insert into detail values(?,?,0,0,0,0,0,0)");
    query.addBindValue(Info.from);
    query.addBindValue(Info.bTime.date());
    query.exec();

    query.prepare("update detail set serve_time = ?,cost = ?,open_times = ?,temp_times = ?,speed_times = ?,dispatch_times = ? where room_id=? and day=?");
    query.addBindValue(serveTime);
    query.addBindValue(Info.cost);
    query.addBindValue(Info.open_times);
    query.addBindValue(Info.temp_times);
    query.addBindValue(Info.speed_times);
    query.addBindValue(Info.dispatch_times);
    query.addBindValue(Info.from);
    query.addBindValue(Info.bTime.date());
    query.exec();
}
#endif // CONNECTION_H

