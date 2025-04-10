#pragma once
#include "BaseImageAI.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

class TongYi_VL_Max : public BaseImageAI
{
    Q_OBJECT
protected slots:
    void imageInfoChanged(ImageInfo &imageInfo) override;
    void replyFinished();

public:
    TongYi_VL_Max();
    TongYi_VL_Max(QList<ImageInfo> &imageList, const QString &token);
    ~TongYi_VL_Max();
    void start() override;
    void stop() override;
    static QJsonObject buildMessage(const QString &role, const QString &type, const QByteArray &data);

private:
    QNetworkAccessManager *_manager;
    QMap<QString, ImageInfo> _replyMap;
    QUrl _url = QUrl("https://dashscope.aliyuncs.com/compatible-mode/v1/chat/completions");
};