#include "BaseImageAI.h"

void BaseImageAI::imageInfoChanged(ImageInfo &imageInfo)
{
    emit imageInfoChangedSignal(imageInfo, ++_progress);
}

BaseImageAI::BaseImageAI()
    : _imageList(QList<ImageInfo>()), _token("")
{
}

BaseImageAI::BaseImageAI(QList<ImageInfo> &imageList, const QString &token)
    : _imageList(imageList), _token(token)
{
}

BaseImageAI::~BaseImageAI()
{
}

void BaseImageAI::reSet(QList<ImageInfo> &imageList)
{
    this->stop();
    _imageList = imageList;
}

bool BaseImageAI::isRunning() const
{
    return _isRunning;
}

int BaseImageAI::progress() const
{
    return _progress;
}

void BaseImageAI::setSystemMessage(const QString &message)
{
    _systemMessage = message;
}

QString BaseImageAI::systemMessage() const
{
    return _systemMessage;
}

QString BaseImageAI::model() const
{
    return _model;
}

void BaseImageAI::setModel(const QString &model)
{
    _model = model;
}

QString BaseImageAI::token() const
{
    return _token;
}

void BaseImageAI::setToken(const QString &token)
{
    _token = token;
}
