#include "ChatFrame.h"
#include <QDesktopServices>
#include <QLayout>
#include <QTimer>
#include <QClipboard>
#include <QFileDialog>
#include <QMovie>
/*
void Document::setText(const QString &text)
{
    if (text == m_text)
        return;
    m_text = text;
    emit textChanged(m_text);
}

void Document::addText(const QString &text)
{
    setText(m_text + text);
    emit textChanged(m_text);
}

bool PreviewPage::acceptNavigationRequest(const QUrl &url,
                                          QWebEnginePage::NavigationType ,
                                          bool )
{
    // Only allow qrc:/index.html.
    if (url.scheme() == QString("qrc"))
        return true;
    QDesktopServices::openUrl(url);
    return false;
}
*/
ChatFrame::ChatFrame(Role role, QWidget *parent)
    : QFrame(parent),
      _role(role)
{
    this->initUi();
    this->initConnect();
}

ChatFrame::~ChatFrame()
{
}

void ChatFrame::setReasonerText(const QString &text)
{
    _reasonerText = text;
    _reasonerTextEdit->setMarkdown(text);
}

void ChatFrame::setChatText(const QString &text)
{
    _chatText = text;
    _chatTextEdit->setMarkdown(text);
}

void ChatFrame::addReasonerText(const QString &text)
{
    _reasonerText += text;
    _reasonerTextEdit->setMarkdown(_reasonerText);
}

void ChatFrame::addChatText(const QString &text)
{
    _chatText += text;
    _chatTextEdit->setMarkdown(_chatText);
}

void ChatFrame::setTokenSize(int size)
{
    if (size <= 0)
        return;
    _tokenSize = size;
    _tokenLabel->setText(QString("Use Token: %1").arg(size));
}

void ChatFrame::startLoading()
{
    _loadingLabel->show();
    _stopButton->show();
    _movie->start();
}

void ChatFrame::stopLoading()
{
    _loadingLabel->hide();
    _stopButton->hide();
    _movie->stop();
}

void ChatFrame::setDateTime(const QString &dateTime)
{
    _dateTime = dateTime;
    _dateTimeLabel->setText(dateTime);
}

void ChatFrame::setErrorText(const QString &text)
{
    _errorLabel->setText(text);
}

void ChatFrame::initUi()
{
    // 设置边框
    this->setFrameShape(QFrame::StyledPanel);
    QVBoxLayout *_layout = new QVBoxLayout();
    QHBoxLayout *_roleLayout = new QHBoxLayout();
    _copyButton->setFixedSize(25, 25);
    _saveButton->setFixedSize(25, 25);
    _copyButton->setIcon(QIcon(":/res/icon/copy.png"));
    _saveButton->setIcon(QIcon(":/res/icon/save.png"));
    _copyButton->setToolTip("复制");
    _saveButton->setToolTip("保存为MD文件");
    _roleLabel->setAlignment(Qt::AlignCenter);
    _tokenLabel->setAlignment(Qt::AlignCenter);
    _loadingLabel->setFixedSize(25, 25);
    _movie->setScaledSize(QSize(50, 50));
    _loadingLabel->setMovie(_movie);
    _loadingLabel->hide();
    _loadingLabel->setAlignment(Qt::AlignCenter);
    _stopButton->setFixedSize(25, 25);
    _stopButton->setIcon(QIcon(":/res/icon/stop.png"));
    _stopButton->setToolTip("停止");
    _stopButton->hide();
    _dateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    _dateTimeLabel->setText(_dateTime);
    _errorLabel->setAlignment(Qt::AlignCenter);
    _errorLabel->setStyleSheet("color:red;");
    if (_role == Role::User)
    {
        _dateTimeLabel->setAlignment(Qt::AlignLeft);
        _roleLabel->setPixmap(QPixmap(":/res/icon/user.png").scaled(20, 20));
        _roleLayout->addWidget(_dateTimeLabel);
        _roleLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
        _roleLayout->addWidget(_errorLabel);
        _roleLayout->addWidget(_stopButton);
        _roleLayout->addWidget(_loadingLabel);
        _roleLayout->addWidget(_tokenLabel);
        _roleLayout->addWidget(_copyButton);
        _roleLayout->addWidget(_saveButton);
        _roleLayout->addWidget(_roleLabel);
    }
    else
    {
        _roleLabel->setPixmap(QPixmap(":/res/icon/DeepSeekPlugin.png").scaled(20, 20));
        _roleLayout->addWidget(_roleLabel);
        _roleLayout->addWidget(_copyButton);
        _roleLayout->addWidget(_saveButton);
        _roleLayout->addWidget(_loadingLabel);
        _roleLayout->addWidget(_stopButton);
        _roleLayout->addWidget(_tokenLabel);
        _roleLayout->addWidget(_errorLabel);
        _roleLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
        _roleLayout->addWidget(_dateTimeLabel);
    }
    _oneRowWidget->setFixedHeight(30);
    _oneRowWidget->setContentsMargins(0, 0, 0, 0);
    _oneRowWidget->setLayout(_roleLayout);
    _roleLayout->setContentsMargins(0, 0, 0, 0);
    _layout->addWidget(_oneRowWidget);
    // 启用自动换行
    _reasonerTextEdit->setWordWrapMode(QTextOption::WordWrap);

    // 移除纵向和横向滚动条
    _reasonerTextEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _reasonerTextEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _chatTextEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _chatTextEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    _reasonerTextEdit->setReadOnly(true);
    _chatTextEdit->setReadOnly(true);
    _reasonerTextEdit->setFixedHeight(0);
    _chatTextEdit->setFixedHeight(0);
    _layout->addWidget(_reasonerTextEdit);
    _layout->addWidget(_chatTextEdit);
    _layout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
    this->setLayout(_layout);
}

void ChatFrame::initConnect()
{
    // 根据文本内容调整高度
    connect(_reasonerTextEdit, &QTextEdit::textChanged, [this]()
            { QTimer::singleShot(100, this, [=]()
                                 { _reasonerTextEdit->setFixedHeight(_reasonerTextEdit->document()->size().height()); }); });
    connect(_chatTextEdit, &QTextEdit::textChanged, [this]()
            { QTimer::singleShot(100, this, [=]()
                                 { _chatTextEdit->setFixedHeight(_chatTextEdit->document()->size().height()); }); });

    connect(_copyButton, &QPushButton::clicked, [this]()
            { QClipboard *clipboard = QGuiApplication::clipboard();
              clipboard->setText(_reasonerTextEdit->toPlainText() + "\n" + _chatTextEdit->toPlainText()); });
    connect(_saveButton, &QPushButton::clicked, [this]()
            {
        QString fileName = QFileDialog::getSaveFileName(this, "保存文件", "", "*.md");
        if (!fileName.isEmpty())
        {
            QFile file(fileName);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                QTextStream out(&file);
                out << this->reasonerText() << "\n" << this->chatText();
                file.close();
            }
        } });
    connect(_stopButton, &QPushButton::clicked, [=]()
            { emit stopRequestSignal(); });
}
