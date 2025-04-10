#include "TongYi_VL_Max.h"
#include <QUuid>

void TongYi_VL_Max::imageInfoChanged(ImageInfo &imageInfo)
{
    BaseImageAI::imageInfoChanged(imageInfo);
}

void TongYi_VL_Max::replyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    auto imageInfo = _replyMap.value(reply->objectName());
    if (reply == nullptr)
        return;
    auto error = reply->error();
    if (error == QNetworkReply::NoError)
    {
        QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
        QJsonObject obj = document.object();

        QString content = obj.value("choices").toArray().at(0).toObject().value("message").toObject().value("content").toString();
        imageInfo.setNewFileName(content);
    }
    else
    {
        int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        imageInfo.setMessage(QString::number(code) + " " + reply->errorString() + " " + reply->readAll());
    }
    _replyMap.remove(reply->objectName());
    reply->deleteLater();
    this->imageInfoChanged(imageInfo);
}

TongYi_VL_Max::TongYi_VL_Max()
    : BaseImageAI()
{
    _manager = new QNetworkAccessManager(this);
    this->setModel("qwen-vl-max-latest");
}

TongYi_VL_Max::TongYi_VL_Max(QList<ImageInfo> &imageList, const QString &token)
    : BaseImageAI(imageList, token)
{
    _manager = new QNetworkAccessManager(this);
    this->setModel("qwen-vl-max-latest");
}

TongYi_VL_Max::~TongYi_VL_Max()
{
    this->stop();
}

void TongYi_VL_Max::start()
{
    _isRunning = true;
    _progress = 0;
    for (auto imageInfo : _imageList)
    {
        QNetworkRequest request = QNetworkRequest(_url);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        request.setRawHeader("Authorization", QString("Bearer %1").arg(_token).toUtf8());
        QJsonObject data;
        data.insert("model", _model);
        QJsonArray messages;
        messages.append(buildMessage("system", "text", _systemMessage.toUtf8()));
        messages.append(buildMessage("user", "image_url", imageInfo.base64Data()));
        messages.append(buildMessage("user", "text", imageInfo.prompt().toUtf8()));
        data.insert("messages", messages);
        QNetworkReply *reply = _manager->post(request, QJsonDocument(data).toJson());
        reply->setObjectName(QUuid::createUuid().toString());
        _replyMap.insert(reply->objectName(), imageInfo);
        connect(reply, &QNetworkReply::finished, this, &TongYi_VL_Max::replyFinished);
        qDebug() << "post" << data;
    }
}

void TongYi_VL_Max::stop()
{
    for (auto uuid : _replyMap.keys())
    {
        // 根据uuid找到reply，然后调用deleteLater()，最后断开连接
        QNetworkReply *reply = _manager->findChild<QNetworkReply *>(uuid);
        disconnect(reply, &QNetworkReply::finished, this, &TongYi_VL_Max::replyFinished);
        reply->abort();
        reply->deleteLater();
    }
    _replyMap.clear();
    _isRunning = false;
}

QJsonObject TongYi_VL_Max::buildMessage(const QString &role, const QString &type, const QByteArray &data)
{
    QJsonObject message;
    message.insert("role", role);
    QJsonArray contentArray;
    QJsonObject content;
    content.insert("type", type);
    if (type == "image_url")
    {
        QJsonObject image_url;
        image_url.insert("url", QString::fromUtf8(data));
        content.insert(type, image_url);
    }
    else
    {
        content.insert(type, QString::fromUtf8(data));
    }
    contentArray.append(content);
    message.insert("content", contentArray);
    return message;
}