#pragma once
#include "ImageInfo.h"
#include <QList>
#include <QObject>

class BaseImageAI : public QObject
{
    Q_OBJECT
protected:
    QList<ImageInfo> _imageList;
    bool _isRunning = false;
    int _progress = 0;
    QString _systemMessage = R"(
请根据我提供的图片和以下信息，按照规范生成照片名称。​​
​我的输入内容：​​


​必填信息​
[日期]：拍摄日期（格式：YYYY-MM-DD，如 2023-01-01）​如果为空，则是未知日期。​​
[地点]：拍摄地点，如果为空，则是未知地点。​​
​你的任务：​​
将时间转为YYMMDD格式，如 20230101。​​
根据图片内容补充主题细节​。
按以下格式生成名称：
{日期}_{主题细节}_{地点}


确保名称无特殊符号。)";
    QString _model;
    QString _token;
signals:
    void imageInfoChangedSignal(const ImageInfo &imageInfo, int progress);

protected slots:
    virtual void imageInfoChanged(ImageInfo &imageInfo);

public:
    BaseImageAI();
    BaseImageAI(QList<ImageInfo> &imageList, const QString &token);
    ~BaseImageAI();
    void reSet(QList<ImageInfo> &imageList);
    virtual void start() = 0;
    virtual void stop() = 0;
    bool isRunning() const;
    int progress() const;
    void setSystemMessage(const QString &message);
    QString systemMessage() const;
    QString model() const;
    void setModel(const QString &model);
    QString token() const;
    void setToken(const QString &token);
};
