#include "DeepSeek.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QEventLoop>
#define SYSTEMROLE "system"
#define USERROLE "user"
#define ASSISTANTROLE "assistant"

QString DeepSeek::errorCodeToString(const ErrorCode &code)
{
    switch (code)
    {
    case NoError:
        return "无错误";
    case FormatError:
        return "格式错误";
    case AuthError:
        return "认证失败";
    case BalanceError:
        return "余额不足";
    case ParameterError:
        return "参数错误";
    case SpeedError:
        return "请求速度超过限制";
    case ServerError:
        return "服务器错误";
    case ServerBusy:
        return "服务器繁忙";
    default:
        return "未知错误";
    }
}

DeepSeek::DeepSeek(const QString &token, QObject *parent)
    : QObject(parent), _token(token)
{
}

DeepSeek::~DeepSeek()
{
}

void DeepSeek::setFrequencyPenalty(double frequency_penalty)
{
    if (frequency_penalty < -2.0 || frequency_penalty > 2.0)
        return;
    _frequency_penalty = frequency_penalty;
}
void DeepSeek::readStream()
{
    _mutex.lock();
    if (!_isRequesting)
        return;
    _mutex.unlock();
    if (!_reply->isOpen())
        return;
    if (_reply->error() != QNetworkReply::NoError)
        return;
    auto data = _reply->readAll();
    QStringList textList = QString::fromUtf8(data).split("data: ");
    emit replyByteArray(data);
    for (const auto &t : textList)
    {
        QJsonDocument doc = QJsonDocument::fromJson(t.toUtf8());
        if (doc.isNull() || doc.isEmpty())
            continue;
        auto obj = doc.object();
        if (obj.contains("usage"))
            _usage = Usage(obj.value("usage").toObject());
        auto message = Message(obj);
        emit replyStreamMessage(message);
    }
}
void DeepSeek::replyFinished_()
{
    QMutexLocker locker(&_mutex);
    _isRequesting = false;
    auto code = _reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    ErrorCode errorCode = static_cast<ErrorCode>(code);
    auto data = _reply->readAll();
    if (errorCode != NoError)
    {
        emit replyFinished(QNetworkReply::NoError, code, errorCodeToString(errorCode));
        goto end;
    }
    if (_reply->error() != QNetworkReply::NoError && _reply->error() != QNetworkReply::OperationCanceledError)
    {
        emit replyFinished(_reply->error(), code, _reply->errorString());
        goto end;
    }
    if (_reply->error() == QNetworkReply::OperationCanceledError)
    {
        emit replyFinished(QNetworkReply::NoError, code, "");
        goto end;
    }
    if (!_isStream)
    {
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isNull() || doc.isEmpty())
            return;
        auto obj = doc.object();
        if (obj.contains("usage"))
            _usage = Usage(obj.value("usage").toObject());
        emit replyMessage(Message(obj));
        emit replyByteArray(data);
    }
    emit replyFinished(QNetworkReply::NoError, code, "");
end:
    _reply->deleteLater();
    _reply = nullptr;
}

QJsonObject DeepSeek::messageToJson(const QString &role, const QString &content)
{
    return QJsonObject{{"role", role}, {"content", content}};
}

void DeepSeek::setMaxTokens(int max_tokens)
{
    if (!DeepSeek::verifyMaxTokens(max_tokens))
        return;
    _max_tokens = max_tokens;
}

void DeepSeek::setStopStrings(const QStringList &stopStrings)
{
    if (stopStrings.size() > 16)
        return;
    _stopStrings = stopStrings;
}

void DeepSeek::setTemperature(double temperature)
{
    if (!DeepSeek::verifyTemperature(temperature))
        return;
    _temperature = temperature;
}

void DeepSeek::setTopP(double top_p)
{
    if (!DeepSeek::verifyTopP(top_p))
        return;
    _top_p = top_p;
}

void DeepSeek::seedMessage(const QList<Message> &oldMessages, const QString &message)
{
    QMutexLocker locker(&_mutex);
    if (_isRequesting)
        return;
    _usage = Usage();
    _isRequesting = true;
    QNetworkRequest request;
    request.setUrl(QUrl("https://api.deepseek.com/chat/completions"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Accept", "application/json");
    request.setRawHeader("Authorization", ("Bearer " + _token).toUtf8());
    QJsonObject obj;
    obj.insert("model", _model);
    obj.insert("frequency_penalty", _frequency_penalty);
    obj.insert("max_tokens", _max_tokens);
    obj.insert("presence_penalty", _presence_penalty);
    if (!_stopStrings.isEmpty())
        obj.insert("stop", QJsonArray::fromStringList(_stopStrings));
    else
        obj.insert("stop", QJsonValue::Null);
    obj.insert("temperature", _temperature);
    obj.insert("top_p", _top_p);
    obj.insert("stream", _isStream);
    obj.insert("response_format", QJsonObject{{"type", "text"}});
    obj.insert("stream_options", QJsonValue::Null);
    obj.insert("tools", QJsonValue::Null);
    obj.insert("tool_choice", "none");
    obj.insert("logprobs", false);
    obj.insert("top_logprobs", QJsonValue::Null);
    QJsonArray messages;
    messages.append(messageToJson(SYSTEMROLE, _system_messages));
    for (auto &oldMessage : oldMessages)
        messages.append(oldMessage.toJson());
    messages.append(messageToJson(USERROLE, message));
    obj.insert("messages", messages);
    auto json = QJsonDocument(obj).toJson(QJsonDocument::Compact);
    _reply = _manager.post(request, json);
    if (_isStream)
    {
        connect(_reply, &QNetworkReply::readyRead, this, &DeepSeek::readStream);
    }
    connect(_reply, &QNetworkReply::finished, this, &DeepSeek::replyFinished_);
}

DeepSeek::Message::Message(QJsonObject obj)
{
    auto choices = obj.value("choices").toArray();
    auto choice = choices.at(0).toObject();
    index = choice.value("index").toInt();
    if (choice.contains("message"))
    {
        role = choice.value("message").toObject().value("role").toString();
        content = choice.value("message").toObject().value("content").toString();
        reasoning_content = choice.value("message").toObject().value("reasoning_content").toString();
        finish_reason = choice.value("finish_reason").toString() == "stop";
    }
    else
    {
        role = ASSISTANTROLE;
        auto _content = choice.value("delta").toObject().value("content");
        auto _reasoning_content = choice.value("delta").toObject().value("reasoning_content");
        finish_reason = !(!_reasoning_content.isNull() && _content.isNull());
        content = _content.isNull() ? "" : _content.toString();
        reasoning_content = _reasoning_content.isNull() ? "" : _reasoning_content.toString();
    }
}

QJsonObject DeepSeek::Message::toJson() const
{
    return QJsonObject{{"role", role}, {"content", content}};
}
QJsonObject DeepSeek::Message::toAllJson() const
{
    return QJsonObject{{"role", role}, {"content", content}, {"reasoning_content", reasoning_content}};
}
void DeepSeek::setPresencePenalty(double presence_penalty)
{
    if (!DeepSeek::verifyPresencePenalty(presence_penalty))
        return;
    _presence_penalty = presence_penalty;
}
void DeepSeek::queryBalance()
{
    QNetworkRequest request;
    request.setUrl(QUrl("https://api.deepseek.com/user/balance"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Accept", "application/json");
    request.setRawHeader("Authorization", ("Bearer " + _token).toUtf8());
    QNetworkReply *reply = _manager.get(request);
    connect(reply, &QNetworkReply::finished, [=]()
            {
            QNetworkReply::NetworkError error = reply->error();
            if (error == QNetworkReply::NoError)
            {
                QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
                QJsonObject obj = doc.object();
                QJsonObject info = obj.value("balance_infos").toArray()[0].toObject();
                Balance balance;
                balance.is_available = obj.value("is_available").toBool();
                balance.currency = info.value("currency").toString();
                balance.total_balance = info.value("total_balance").toString().toDouble();
                balance.granted_balance = info.value("granted_balance").toString().toDouble();
                balance.topped_up_balance = info.value("topped_up_balance").toString().toDouble();
                emit replyBalance(balance);
                reply->deleteLater();
                return;
            }
            emit replyBalance(Balance());
            reply->deleteLater(); });
}

void DeepSeek::stopRequest()
{
    if (!_isRequesting)
        return;
    _reply->abort();
}

bool DeepSeek::verifyTopP(double top_p)
{
    return top_p >= 0.0 && top_p <= 1.0;
}

bool DeepSeek::verifyTemperature(double temperature)
{
    return temperature >= 0.0 && temperature <= 2.0;
}

bool DeepSeek::verifyMaxTokens(int max_tokens)
{
    return max_tokens >= 1 && max_tokens <= 8192;
}

bool DeepSeek::verifyFrequencyPenalty(double frequency_penalty)
{
    return frequency_penalty >= -2.0 && frequency_penalty <= 2.0;
}

bool DeepSeek::verifyPresencePenalty(double presence_penalty)
{
    return presence_penalty >= -2.0 && presence_penalty <= 2.0;
}

QStringList DeepSeek::models()
{
    QNetworkRequest request;
    request.setUrl(QUrl("https://api.deepseek.com/models"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Accept", "application/json");
    request.setRawHeader("Authorization", ("Bearer " + _token).toUtf8());
    QNetworkReply *reply = _manager.get(request);
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    auto error = reply->error();
    if (error == QNetworkReply::NoError)
    {
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonObject obj = doc.object();
        QJsonArray models = obj.value("data").toArray();
        QStringList list;
        for (int i = 0; i < models.size(); i++)
            list.append(models.at(i).toObject().value("id").toString());
        reply->deleteLater();
        return list;
    }
    reply->deleteLater();
    return {"deepseek-chat", "deepseek-reasoner"};
}
