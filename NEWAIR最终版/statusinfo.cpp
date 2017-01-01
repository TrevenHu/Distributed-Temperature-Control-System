#include "statusinfo.h"

StatusInfo::StatusInfo()
{
}

StatusInfo::StatusInfo(QString json)
{
    QJsonParseError error;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(json.toUtf8(), &error);
    if (error.error == QJsonParseError::NoError) {
        if (jsonDocument.isObject()) {
            QVariantMap result = jsonDocument.toVariant().toMap();

            from = result["From"].toInt();
            type = result["Type"].toString() ;

            QVariantMap nestedMap = result["Action"].toMap();
            bTemp = cTemp = nestedMap["Ctemp"].toInt();
            dTemp = nestedMap["Dtemp"].toInt();
            speed = nestedMap["speed"].toString();
            workmode = nestedMap["workmode"].toString();
            bTime = QDateTime::currentDateTime();
            status = 1;
            serveTime = 0;
            cost = 0;
            open_times = 0;
            temp_times = 0;
            speed_times = 0;
            dispatch_times = 0;
        }
    } else {
        qFatal(error.errorString().toUtf8().constData());
        exit(1);
    }
}

bool StatusInfo::is_changed(StatusInfo last)
{
    if(this->dTemp == last.dTemp && this->speed == last.speed && this->workmode == last.workmode)
        return false;
    else
        return true;
}

int change(QString speed)
{
    if(speed == "High")
        return 3;
    else if(speed == "Medium")
        return 2;
    else if(speed == "Low")
        return 1;
    return 0;
}

bool StatusInfo::is_high(StatusInfo S)
{
    if(this->speed != S.speed)
        return change(this->speed) > change(S.speed);
    if(this->serveTime != S.serveTime)
        return this->serveTime < S.serveTime;
    return this->lTime > S.lTime;
}

//0表示不是请求，1表示风速，2表示温度
int StatusInfo::get_req(StatusInfo S)
{
    if(this->is_changed(S))
    {
        if(this->speed != S.speed)
            return 1;
        return 2;
    }
    return 0;
}

void StatusInfo::print()
{
    qDebug()<<from<<type<<cTemp<<dTemp<<speed<<workmode<<bTime<<status<<lTime<<serveTime;
}





