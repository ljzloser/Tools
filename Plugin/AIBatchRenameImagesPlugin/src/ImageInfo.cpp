#include "ImageInfo.h"
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QBuffer>

ImageInfo::ImageInfo()
{
}

ImageInfo::ImageInfo(const QString &filePath, int fileSize)
    : ImageInfo()
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
                // 1. 预计算
                const double ratio = static_cast<double>(fileSize) / oldFileSize;
                const int targetWidth = image.width() * ratio;
                const int targetHeight = image.height() * ratio;

                // 2. 智能选择缩放算法
                auto transform = (ratio < 1.0) ? Qt::SmoothTransformation : // 缩小用高质量
                                     Qt::FastTransformation;                // 放大用快速算法

                // 3. 内存预分配
                QByteArray byteArray;
                byteArray.reserve(targetWidth * targetHeight * 3 / 4); // 保守估计

                // 4. 组合操作
                {
                    QBuffer buffer(&byteArray);
                    if (buffer.open(QIODevice::WriteOnly))
                    {
                        image.scaled(targetWidth, targetHeight,
                                     Qt::KeepAspectRatio, transform)
                            .save(&buffer, "JPEG", 90); // 适当质量
                    }
                }
                QString type = oldFileName().split(".").last().toLower();
                QString Content_Type;
                if (type == "bmp" || type == "dib")
                {
                    Content_Type = "image/bmp";
                }
                else if (type == "icns")
                {
                    Content_Type = "image/icns";
                }
                else if (type == "ico")
                {
                    Content_Type = "image/x-icon";
                }
                else if (type == "jpeg" || type == "jpg" || type == "jpe" || type == "jif")
                {
                    Content_Type = "image/jpeg";
                }
                else if (type == "j2c" || type == "j2k" || type == "jp2" || type == "jpc" || type == "jpx" || type == "jpf")
                {
                    Content_Type = "image/jp2";
                }
                else if (type == "png" || type == "apng")
                {
                    Content_Type = "image/png";
                }
                else if (type == "bw" || type == "rgb" || type == "rgba" || type == "sgi")
                {
                    Content_Type = "image/sgi";
                }
                else if (type == "tif" || type == "tiff")
                {
                    Content_Type = "image/tiff";
                }
                else if (type == "webp")
                {
                    Content_Type = "image/webp";
                }
                QString base64Data = "data:" + Content_Type + ";base64," + byteArray.toBase64();
                // 5. Base64编码
                _base64Data = base64Data.toUtf8();
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

QString ImageInfo::message() const
{
    return _message;
}

void ImageInfo::setMessage(const QString &message)
{
    _message = message;
}

QString ImageInfo::prompt() const
{
    return _prompt;
}

void ImageInfo::setPrompt(const QString &prompt)
{
    _prompt = prompt;
}
