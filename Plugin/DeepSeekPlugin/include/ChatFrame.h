#pragma once
#include <QFrame>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListWidget>
#include <QListWidgetItem>
#include <QLineEdit>
#include <QPushButton>
#include <QWidget>
#include <QPlainTextEdit>
#include <QMovie>

enum class Role
{
    User,
    Assistant
};
static QString roleToString(Role role)
{
    switch (role)
    {
    case Role::User:
        return "user";
    case Role::Assistant:
        return "assistant";
    default:
        return "";
    }
}
static Role stringToRole(QString role)
{
    if (role == "user")
    {
        return Role::User;
    }
    else if (role == "assistant")
    {
        return Role::Assistant;
    }
    return Role::User;
}
/*
class Document : public QObject
{
Q_OBJECT
Q_PROPERTY(QString text MEMBER m_text NOTIFY textChanged FINAL)
public:
explicit Document(QObject *parent = nullptr) : QObject(parent) {}

void setText(const QString &text);
void addText(const QString &text);
signals:
void textChanged(const QString &text);

private:
QString m_text;

class PreviewPage : public QWebEnginePage
{
    Q_OBJECT
public:
    explicit PreviewPage(QObject *parent = nullptr) : QWebEnginePage(parent) {}

protected:
    bool acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame);
};
*/
class ChatFrame : public QFrame
{
    Q_OBJECT
public:
    ChatFrame(Role role, QWidget *parent = nullptr);
    ~ChatFrame();
public slots:
    void setReasonerText(const QString &text);
    void setChatText(const QString &text);
    void addReasonerText(const QString &text);
    void addChatText(const QString &text);
    QString role() const { return roleToString(_role); }
    QString chatText() const { return _chatText; }
    QString reasonerText() const { return _reasonerText; }
    void setTokenSize(int size);
    void startLoading();
    void stopLoading();
    int tokenSize() const { return _tokenSize; }
    QString dateTime() const { return _dateTime; }
    void setDateTime(const QString &dateTime);
    void setErrorText(const QString &text);

signals:
    void stopRequestSignal();

private:
    void initUi();
    void initConnect();

private:
    Role _role;
    QLabel *_roleLabel = new QLabel(this);
    QWidget *_oneRowWidget = new QWidget(this);
    QTextEdit *_reasonerTextEdit = new QTextEdit(this);
    QTextEdit *_chatTextEdit = new QTextEdit(this);
    QString _reasonerText = "";
    QString _chatText = "";
    QPushButton *_copyButton = new QPushButton(this);
    QPushButton *_saveButton = new QPushButton(this);
    QLabel *_tokenLabel = new QLabel(this);
    QLabel *_loadingLabel = new QLabel(this);
    QMovie *_movie = new QMovie(":/res/icon/loading.gif");
    QPushButton *_stopButton = new QPushButton(this);
    int _tokenSize = 0;
    QString _dateTime;
    QLabel *_dateTimeLabel = new QLabel(this);
    QLabel *_errorLabel = new QLabel(this);
};