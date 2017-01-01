#ifndef STATUSINFO_H
#define STATUSINFO_H
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QDateTime>
#include <QVariantMap>
#include <QDebug>
class StatusInfo
{
public:
    StatusInfo();
    StatusInfo(QString json);
    bool is_changed(StatusInfo last);
    bool is_high(StatusInfo S);
    int get_req(StatusInfo S);
    void print();
public:
    int from; //0-3
    QString type;
    int cTemp;
    int dTemp;
    QString speed;
    QString workmode;
    QDateTime bTime;
    int status; //0表示已完成，1表示等待，2表示服务中，3表示关机
    QDateTime lTime;//最近一次被服务的开始时间
    int serveTime; //按秒计算
    float cost;
    int bTemp;
    int open_times;
    int temp_times;
    int speed_times;
    int dispatch_times;
};
#endif // STATUSINFO_H
