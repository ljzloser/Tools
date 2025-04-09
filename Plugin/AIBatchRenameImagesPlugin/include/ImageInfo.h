#pragma once
#include <QString>

class ImageInfo
{
public:
    ImageInfo(const QString &filePath, int fileSize = 1);
    ~ImageInfo();
    QString fileDir() const;
    QString oldFileName() const;
    QString newFileName() const;
    QByteArray base64Data() const;
    void setNewFileName(const QString &newFileName);
    bool isValid() const;

private:
    QString _fileDir;
    QString _oldFileName;
    QString _newFileName;
    QByteArray _base64Data;
    int _fileSize;
};