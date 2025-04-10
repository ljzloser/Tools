#pragma once
#include <QString>

class ImageInfo
{
public:
    ImageInfo();
    ImageInfo(const QString &filePath, int fileSize = 1);
    ~ImageInfo();
    QString fileDir() const;
    QString oldFileName() const;
    QString newFileName() const;
    QByteArray base64Data() const;
    void setNewFileName(const QString &newFileName);
    bool isValid() const;
    QString message() const;
    void setMessage(const QString &message);
    QString prompt() const;
    void setPrompt(const QString &prompt);

private:
    QString _fileDir;
    QString _oldFileName;
    QString _newFileName;
    QByteArray _base64Data;
    int _fileSize;
    QString _message;
    QString _prompt;
};