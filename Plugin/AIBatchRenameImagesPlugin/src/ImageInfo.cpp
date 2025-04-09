#include "ImageInfo.h"
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QBuffer>

ImageInfo::ImageInfo(const QString &filePath, int fileSize)
{
    QFileInfo fileInfo(filePath);
    if (fileInfo.exists())
    {
        _fileDir = fileInfo.absolutePath();
        _oldFileName = fileInfo.fileName();
        int oldFileSize = fileInfo.size() / 1024 / 1024;
        QImage image(filePath);
        if (!image.isNull())
        {
            if (oldFileSize > fileSize)
            {
                QImage scaledImage = image.scaled(image.width() * fileSize / oldFileSize, image.height() * fileSize / oldFileSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                QByteArray byteArray;
                QBuffer buffer(&byteArray);
                buffer.open(QIODevice::WriteOnly);
                scaledImage.save(&buffer, "JPEG", 100);
                buffer.close();
                _base64Data = byteArray.toBase64();
            }
        }
    }
    else
    {
        _fileDir = "";
        _oldFileName = "";
    }
}

ImageInfo::~ImageInfo()
{
}

QString ImageInfo::fileDir() const
{
    return _fileDir;
}

QString ImageInfo::oldFileName() const
{
    return _oldFileName;
}

QString ImageInfo::newFileName() const
{
    return _newFileName;
}

QByteArray ImageInfo::base64Data() const
{
    return _base64Data;
}

void ImageInfo::setNewFileName(const QString &newFileName)
{
    _newFileName = newFileName;
}

bool ImageInfo::isValid() const
{
    return _base64Data.size() > 0;
}
