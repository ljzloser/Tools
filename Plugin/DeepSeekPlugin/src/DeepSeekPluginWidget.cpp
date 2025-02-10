#include "DeepSeekPluginWidget.h"
#include <QMessageBox>
#include <QImage>
#include <QIcon>
#include <qdesktopservices.h>
#include <QScrollBar>
#include <QTimer>
#include <QMenu>
#include <QAction>
#include <QUuid>
#include "DeepSeekModel.h"
DeepSeekWidget::DeepSeekWidget(Logger *logger, TConfig *config, QWidget *parent)
    : QWidget(parent), ui(new Ui::DeepSeekPluginWidget()), _config(config), _logger(logger)
{
    // _sqlExecutor->executeNonQuery(R"(CREATE TABLE IF NOT EXISTS ChatMessage
    //     (id INTEGER PRIMARY KEY AUTOINCREMENT,identifier TEXT, datetime TEXT, chat_name TEXT, content TEXT, isLegal INTEGER);)");
    DBModeHelper::Create<DeepSeekModel>();
    ui->setupUi(this);
    deepSeek = new DeepSeek(_config->read("token").valueString(), this);
    deepSeek->setSystemMessage(_config->read("system_messages").valueString());
    deepSeek->setMaxTokens(_config->read("max_tokens").value.toInt());
    deepSeek->setTemperature(_config->read("temperature").value.toDouble());
    deepSeek->setTopP(_config->read("top_p").value.toDouble());
    deepSeek->setPresencePenalty(_config->read("presence_penalty").value.toDouble());
    deepSeek->setFrequencyPenalty(_config->read("frequency_penalty").value.toDouble());
    deepSeek->setModel(_config->read("model").value.value<ComboxData>().currentText());
    deepSeek->setStream(_config->read("isStream").value.toBool());
    this->initUi();
    this->initConnect();
    this->loadChat();
}

DeepSeekWidget::~DeepSeekWidget()
{
    delete ui;
    // delete _sqlExecutor;
}
void DeepSeekWidget::setParmas(QString key, QVariant value)
{
    if (key == "system_messages")
    {
        deepSeek->setSystemMessage(value.toString());
    }
    else if (key == "max_tokens")
    {
        deepSeek->setMaxTokens(value.toInt());
    }
    else if (key == "temperature")
    {
        deepSeek->setTemperature(value.toDouble());
    }
    else if (key == "top_p")
    {
        deepSeek->setTopP(value.toDouble());
    }
    else if (key == "presence_penalty")
    {
        deepSeek->setPresencePenalty(value.toDouble());
    }
    else if (key == "frequency_penalty")
    {
        deepSeek->setFrequencyPenalty(value.toDouble());
    }
    else if (key == "model")
    {
        deepSeek->setModel(value.value<ComboxData>().currentText());
    }
    else if (key == "isStream")
    {
        deepSeek->setStream(value.toBool());
    }
    else if (key == "token")
    {
        deepSeek->setToken(value.toString());
    }
}
/**
 * @brief 初始化UI界面最好在这里完成
 */
void DeepSeekWidget::initUi()
{
#pragma region 初始化分割器的尺寸
    ui->hSplitter->setStretchFactor(0, 1);
    ui->hSplitter->setStretchFactor(1, 5);
    ui->vSplitter->setSizes(QList<int>() << 800 << 200);
    ui->vSplitter->setStretchFactor(0, 5);
    ui->vSplitter->setStretchFactor(1, 1);
#pragma endregion 初始化分割器的尺寸
    _spacer = new QWidget();
    _spacer->setFixedHeight(300);
    _mainLayout->addWidget(_spacer);
    ui->chatScrollArea->setWidget(_mainWidget);
    ui->chatScrollArea->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->chatListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    // 多选
    ui->chatListWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->textEdit->setPlaceholderText("请输入你的问题后按下Ctrl+Enter来进行提问!");
}
/**
 * @brief 初始化各种信号和槽最好在这里完成
 */
void DeepSeekWidget::initConnect()
{
    connect(deepSeek, &DeepSeek::replyStreamMessage, this, &DeepSeekWidget::addLastMessage);
    connect(deepSeek, &DeepSeek::replyMessage, this, &DeepSeekWidget::addLastMessage);
    connect(deepSeek, &DeepSeek::replyFinished, this, &DeepSeekWidget::finished);
    connect(ui->chatScrollArea, &QScrollArea::customContextMenuRequested, this, &DeepSeekWidget::showContextMenu);
    connect(ui->newChatButton, &QPushButton::clicked, this, &DeepSeekWidget::newChat);
    connect(ui->chatListWidget, &QListWidget::itemClicked, this, &DeepSeekWidget::loadChatMessage);
    connect(ui->chatListWidget, &QListWidget::customContextMenuRequested, this, &DeepSeekWidget::showListWidgetContextMenu);
}

void DeepSeekWidget::keyPressEvent(QKeyEvent *event)
{
    // Ctrl + Enter
    if (event->key() == Qt::Key_Return && event->modifiers() == Qt::ControlModifier)
    {
        if (deepSeek->isRequesting())
            return;
        auto text = ui->textEdit->toPlainText();
        ChatFrame *chatFrame = new ChatFrame(Role::User, this);
        chatFrame->setChatText(text);
        // 再倒数第一个前面添加
        _mainLayout->insertWidget(_mainLayout->count() - 1, chatFrame);
        // scrollArea 滚动到_spacer上面，也就要要最大值 - _spacer的高度
        // ui->chatScrollArea->verticalScrollBar()->setValue(ui->chatScrollArea->verticalScrollBar()->maximum() - spacer->height());

        ChatFrame *reasonerFrame = new ChatFrame(Role::Assistant, this);
        _mainLayout->insertWidget(_mainLayout->count() - 1, reasonerFrame);
        _logger->info(QString("seed: %1").arg(text));
        deepSeek->seedMessage(oldMessage(), text);
        QTimer::singleShot(500, this, [=]
                           { ui->chatScrollArea->ensureWidgetVisible(reasonerFrame); });
        reasonerFrame->startLoading();
        ui->textEdit->clear();
        ui->textEdit->setEnabled(false);
        if (_mainLayout->count() == 3)
        {
            _identifier = QUuid::createUuid().toString();
            _name = text.size() > 10 ? text.left(10) + "..." : text;
            // auto sql = QString("INSERT INTO ChatMessage (identifier, datetime, chat_name, isLegal) VALUES ('%1', '%2', '%3', 0)")
            //                .arg(_identifier)
            //                .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
            //                .arg(_name);
            // _sqlExecutor->executeNonQuery(sql);
            DeepSeekModel model;
            model.identifier_set(_identifier);
            model.datetime_set(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
            model.chat_name_set(_name);
            model.isLegal_set(false);
            model.Insert();
        }
    }
}

void DeepSeekWidget::addLastMessage(const DeepSeek::Message &message)
{
    auto widget = _mainLayout->itemAt(_mainLayout->count() - 2)->widget();
    if (widget)
    {
        ChatFrame *chatFrame = static_cast<ChatFrame *>(widget);
        if (!message.content.isEmpty())
        {
            chatFrame->addChatText(message.content);
            _logger->info(QString("content: %1").arg(message.content));
        }
        if (!message.reasoning_content.isEmpty())
        {
            chatFrame->addReasonerText(message.reasoning_content);
            _logger->info(QString("reasoning_content: %1").arg(message.reasoning_content));
        }
        ui->chatScrollArea->ensureWidgetVisible(widget);
    }
    // ui->chatScrollArea->verticalScrollBar()->setValue(ui->chatScrollArea->verticalScrollBar()->maximum());
}

void DeepSeekWidget::finished(QNetworkReply::NetworkError error, int httpStatusCode, const QString &errorString)
{
    ui->textEdit->setEnabled(true);
    ui->textEdit->setFocus();

    auto widget = _mainLayout->itemAt(_mainLayout->count() - 2)->widget();
    if (widget)
    {
        ChatFrame *chatFrame = static_cast<ChatFrame *>(widget);
        chatFrame->setTokenSize(deepSeek->lastUsage().total_tokens);
        chatFrame->setErrorText(errorString);
        chatFrame->stopLoading();
    }
    QJsonArray array;
    for (int i = 0; i < _mainLayout->count() - 1; ++i)
    {
        auto chatFrame = static_cast<ChatFrame *>(_mainLayout->itemAt(i)->widget());
        if (chatFrame)
        {
            QJsonObject chat{{"role", chatFrame->role()},
                             {"content", chatFrame->chatText()},
                             {"reasoning_content", chatFrame->reasonerText()},
                             {"total_tokens", chatFrame->tokenSize()},
                             {"datetime", chatFrame->dateTime()}};
            array.append(chat);
        }
    }
    QJsonDocument doc(array);
    QString content = doc.toJson(QJsonDocument::Compact);
    // auto sql = QString("UPDATE ChatMessage SET content = '%1', isLegal = 1 WHERE identifier = '%2'")
    //                .arg(content)
    //                .arg(_identifier);
    // _sqlExecutor->executeNonQuery(sql);
    auto deepSeekModel = DBModeHelper::Fliter<DeepSeekModel>(QString("identifier = '%1'").arg(_identifier)).first();
    deepSeekModel->content_set(content);
    deepSeekModel->isLegal_set(true);
    deepSeekModel->Update();
    if (_mainLayout->count() == 3)
    {
        // 在listWidget中第一行插入一行
        QListWidgetItem *item = new QListWidgetItem();
        item->setText(_name);
        item->setData(Qt::UserRole, _identifier);
        ui->chatListWidget->insertItem(0, item);
    }

    _logger->info("finished");
    _logger->info(QString("total_tokens: %1").arg(deepSeek->lastUsage().total_tokens));
}

QList<DeepSeek::Message> DeepSeekWidget::oldMessage()
{
    auto ms = QList<DeepSeek::Message>();
    for (int i = 0; i < _mainLayout->count() - 1; ++i)
    {
        auto chatFrame = static_cast<ChatFrame *>(_mainLayout->itemAt(i)->widget());
        if (chatFrame)
        {
            ms.append(DeepSeek::Message(chatFrame->role(), chatFrame->chatText()));
        }
    }
    return ms;
}

void DeepSeekWidget::newChat()
{
    // 判断当前的对话是否合法
    // auto sql = QString("SELECT isLegal FROM ChatMessage WHERE identifier = '%1'").arg(_identifier);
    // int isLegal = _sqlExecutor->executeScalar<int>(sql);
    auto models = DBModeHelper::Fliter<DeepSeekModel>(QString("identifier = '%1'").arg(_identifier));
    if (models.size() == 0)
        return;
    auto model = models.first();
    int isLegal = model->isLegal_get();
    if (isLegal == 0)
    {
        // sql = QString("Delete FROM ChatMessage WHERE identifier = '%1'").arg(_identifier);
        // _sqlExecutor->executeNonQuery(sql);
        model->Delete();
    }
    while (_mainLayout->count() > 1)
    {
        auto chatFrame = static_cast<ChatFrame *>(_mainLayout->itemAt(0)->widget());
        if (chatFrame)
        {
            _mainLayout->removeWidget(chatFrame);
            delete chatFrame;
        }
    }
    model->deleteLater();
}

void DeepSeekWidget::loadChat()
{
    ui->chatListWidget->clear();
    // auto sql = QString("SELECT chat_name,identifier FROM ChatMessage where isLegal = 1 order by datetime desc");
    // auto rows = _sqlExecutor->executeQuery(sql);
    auto models = DBModeHelper::Fliter<DeepSeekModel>("isLegal = 1 order by datetime desc");
    for (auto model : models)
    {
        auto identifier = model->identifier_get();
        auto chat_name = model->chat_name_get();
        QListWidgetItem *item = new QListWidgetItem();
        item->setText(chat_name);
        item->setData(Qt::UserRole, identifier);
        ui->chatListWidget->addItem(item);
    }
    models.clear();
}

void DeepSeekWidget::showContextMenu(const QPoint &pos)
{
    QMenu menu(this);
    auto newChatAction = menu.addAction("新聊天");
    connect(newChatAction, &QAction::triggered, this, &DeepSeekWidget::newChat);
    menu.exec(QCursor::pos());
}

void DeepSeekWidget::showListWidgetContextMenu(const QPoint &pos)
{
    QMenu menu(this);
    auto deleteChatAction = menu.addAction("删除");
    connect(deleteChatAction, &QAction::triggered, this, &DeepSeekWidget::deleteChat);
    menu.exec(QCursor::pos());
}

void DeepSeekWidget::loadChatMessage(QListWidgetItem *item)
{
    this->newChat();
    _identifier = item->data(Qt::UserRole).toString();
    // auto sql = QString("SELECT chat_name,identifier,content FROM ChatMessage WHERE identifier = '%1'").arg(_identifier);
    // auto row = _sqlExecutor->executeFirstRow(sql);
    auto model = DBModeHelper::Fliter<DeepSeekModel>(QString("identifier = '%1'").arg(_identifier)).first();
    _identifier = model->identifier_get();
    _name = model->chat_name_get();
    auto content = model->content_get();
    auto doc = QJsonDocument::fromJson(content.toUtf8());
    auto array = doc.array();
    for (auto row : array)
    {
        auto obj = row.toObject();
        auto role = obj.value("role").toString();
        auto content = obj.value("content").toString();
        auto reasoning_content = obj.value("reasoning_content").toString();
        auto datetime = obj.value("datetime").toString();
        auto total_tokens = obj.value("total_tokens").toInt();
        auto chatFrame = new ChatFrame(stringToRole(role), nullptr);
        if (!content.isEmpty())
        {
            chatFrame->addChatText(content);
        }
        if (!reasoning_content.isEmpty())
        {
            chatFrame->addReasonerText(reasoning_content);
        }
        chatFrame->setTokenSize(total_tokens);
        chatFrame->setDateTime(datetime);
        _mainLayout->insertWidget(_mainLayout->count() - 1, chatFrame);
    }
}

void DeepSeekWidget::deleteChat()
{
    auto items = ui->chatListWidget->selectedItems();
    for (auto item : items)
    {
        auto identifier = item->data(Qt::UserRole).toString();
        if (identifier == _identifier)
            newChat();
        // auto sql = QString("Delete FROM ChatMessage WHERE identifier = '%1'").arg(identifier);
        // _sqlExecutor->executeNonQuery(sql);
        auto model = DBModeHelper::Fliter<DeepSeekModel>(QString("identifier = '%1'").arg(identifier)).first();
        model->Delete();
        model->deleteLater();
        ui->chatListWidget->removeItemWidget(item);
        delete item;
    }
}
