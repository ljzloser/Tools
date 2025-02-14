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
#include <QFileDialog>
#include <QInputDialog>

DeepSeekWidget::DeepSeekWidget(Logger *logger, TConfig *config, QWidget *parent)
    : QWidget(parent), ui(new Ui::DeepSeekPluginWidget()), _config(config), _logger(logger)
{
    DBModelHelper::Create<DeepSeekModel>();
    // 清除所有不合法的聊天记录
    auto models = DBModelHelper::Fliter<DeepSeekModel>("isLegal = 0");
    DBModelHelper::DeleteModels(models);
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
    deepSeek->queryBalance();
    _timer->start(60000);
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
    else if (key == "blance_update_interval")
    {
        _timer->stop();
        _timer->start(value.toInt() * 1000);
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
    _mainLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
    ui->chatScrollArea->setWidget(_mainWidget);
    ui->chatScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->chatScrollArea->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->chatListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    _mainLayout->setSpacing(20);
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
    connect(_timer, &QTimer::timeout, deepSeek, &DeepSeek::queryBalance);
    connect(deepSeek, &DeepSeek::replyBalance, this, &DeepSeekWidget::updateBalance);
}

void DeepSeekWidget::keyPressEvent(QKeyEvent *event)
{
    // Ctrl + Enter
    if (event->key() == Qt::Key_Return && event->modifiers() == Qt::ControlModifier)
    {
        if (deepSeek->isRequesting())
            return;
        auto lastChatFrame = this->lastChatFrame();
        if (lastChatFrame)
        {
            disconnect(lastChatFrame, &ChatFrame::stopRequestSignal, deepSeek, &DeepSeek::stopRequest);
        }
        auto text = ui->textEdit->toPlainText();
        ChatFrame *chatFrame = new ChatFrame(Role::User, this);
        chatFrame->setChatText(text);
        // 再倒数第一个前面添加
        _mainLayout->insertWidget(_mainLayout->count() - 1, chatFrame);
        ChatFrame *reasonerFrame = new ChatFrame(Role::Assistant, this);
        _mainLayout->insertWidget(_mainLayout->count() - 1, reasonerFrame);
        _logger->info(QString("seed: %1").arg(text));
        deepSeek->seedMessage(oldMessage(), text);
        this->rollLast();
        reasonerFrame->startLoading();
        ui->textEdit->clear();
        ui->textEdit->setEnabled(false);
        if (_mainLayout->count() == 3)
        {
            _identifier = QUuid::createUuid().toString();
            _name = text.size() > 10 ? text.left(10) + "..." : text;
            DeepSeekModel model;
            model.identifier_set(_identifier);
            model.datetime_set(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
            model.chat_name_set(_name);
            model.isLegal_set(false);
            model.Insert();
        }
        connect(reasonerFrame, &ChatFrame::stopRequestSignal, deepSeek, &DeepSeek::stopRequest);
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
        this->rollLast();
    }
}

void DeepSeekWidget::finished(QNetworkReply::NetworkError error, int httpStatusCode, const QString &errorString)
{
    ui->textEdit->setEnabled(true);
    ui->textEdit->setFocus();

    auto widget = _mainLayout->itemAt(_mainLayout->count() - 2)->widget();
    if (widget)
    {
        ChatFrame *chatFrame = static_cast<ChatFrame *>(widget);
        chatFrame->setUsage(deepSeek->lastUsage());
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
                             {"usage", chatFrame->usage().toJson()},
                             {"datetime", chatFrame->dateTime()}};
            array.append(chat);
        }
    }
    QJsonDocument doc(array);
    QString content = doc.toJson(QJsonDocument::Compact);
    auto deepSeekModel = DBModelHelper::Fliter<DeepSeekModel>(QString("identifier = '%1'").arg(_identifier)).first();
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

    _logger->info("Chat finished");
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
    auto chatFrame = this->lastChatFrame();
    if (!chatFrame)
        return;
    chatFrame->stopLoading();
    deepSeek->stopRequest();

    // 判断当前的对话是否合法
    auto models = DBModelHelper::Fliter<DeepSeekModel>(QString("identifier = '%1'").arg(_identifier));
    if (models.size() == 0)
        return;
    auto model = models.first();
    int isLegal = model->isLegal_get();
    if (isLegal == 0)
    {
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
    this->newChat();
    ui->chatListWidget->clear();
    auto models = DBModelHelper::Fliter<DeepSeekModel>("isLegal = 1");
    std::sort(models.begin(), models.end(), [](QSharedPointer<DeepSeekModel> a, QSharedPointer<DeepSeekModel> b) -> bool
              { return a->datetime_get() > b->datetime_get(); });
    for (auto model : models)
    {
        auto identifier = model->identifier_get();
        auto chat_name = model->chat_name_get();
        QListWidgetItem *item = new QListWidgetItem();
        item->setText(chat_name);
        item->setData(Qt::UserRole, identifier);
        ui->chatListWidget->addItem(item);
        _logger->info(QString("Load Chat identifier: %1 Success").arg(identifier));
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
    auto newChatAction = menu.addAction("新聊天");
    auto deleteChatAction = menu.addAction("删除");
    auto reNameAction = menu.addAction("重命名");
    auto exportAction = menu.addAction("导出");
    auto leadAction = menu.addAction("导入");

    connect(deleteChatAction, &QAction::triggered, this, &DeepSeekWidget::deleteChat);
    connect(newChatAction, &QAction::triggered, this, &DeepSeekWidget::newChat);
    connect(reNameAction, &QAction::triggered, this, &DeepSeekWidget::reNameChat);
    connect(exportAction, &QAction::triggered, this, &DeepSeekWidget::exportChat);
    connect(leadAction, &QAction::triggered, this, &DeepSeekWidget::leadChat);
    menu.exec(QCursor::pos());
}

void DeepSeekWidget::loadChatMessage(QListWidgetItem *item)
{
    this->newChat();
    _identifier = item->data(Qt::UserRole).toString();
    auto model = DBModelHelper::Fliter<DeepSeekModel>(QString("identifier = '%1'").arg(_identifier)).first();
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
        auto usage = DeepSeek::Usage(obj.value("usage").toObject());

        auto chatFrame = new ChatFrame(stringToRole(role), nullptr);
        if (!content.isEmpty())
        {
            chatFrame->addChatText(content);
        }
        if (!reasoning_content.isEmpty())
        {
            chatFrame->addReasonerText(reasoning_content);
        }
        chatFrame->setUsage(usage);
        chatFrame->setDateTime(datetime);
        _mainLayout->insertWidget(_mainLayout->count() - 1, chatFrame);
    }
    this->rollLast();
    _logger->info(QString("Load Chat Message identifier: %1 Success.").arg(_identifier));
}

void DeepSeekWidget::deleteChat()
{
    auto items = ui->chatListWidget->selectedItems();
    for (auto item : items)
    {
        auto identifier = item->data(Qt::UserRole).toString();
        if (identifier == _identifier)
            newChat();
        auto model = DBModelHelper::Fliter<DeepSeekModel>(QString("identifier = '%1'").arg(identifier)).first();
        model->Delete();
        model->deleteLater();
        ui->chatListWidget->removeItemWidget(item);
        _logger->info(QString("delete Chat identifier: %1").arg(identifier));
        delete item;
    }
}

void DeepSeekWidget::rollLast()
{
    QTimer::singleShot(500, this, [=]
                       { ui->chatScrollArea->verticalScrollBar()->setValue(ui->chatScrollArea->verticalScrollBar()->maximum()); });
}

void DeepSeekWidget::updateBalance(DeepSeek::Balance balance)
{
    ui->balanceLabel->setText(QString("当前余额: %1 元").arg(balance.total_balance));
    ui->balanceLabel->setToolTip(QString(R"(是否可用：%1
货币：%2
总余额：%3
未过期赠金余额：%4
充值余额：%5)")
                                     .arg(balance.is_available ? "是" : "否")
                                     .arg(balance.currency)
                                     .arg(balance.total_balance)
                                     .arg(balance.granted_balance)
                                     .arg(balance.topped_up_balance));
    _logger->info(balance.toString());
}

void DeepSeekWidget::exportChat()
{
    auto items = ui->chatListWidget->selectedItems();
    if (items.size() == 0)
    {
        for (int i = 0; i < ui->chatListWidget->count(); i++)
        {
            items.append(ui->chatListWidget->item(i));
        }
    }
    QStringList limits;
    for (auto item : items)
    {
        limits.append(DBModelHelper::addQuotes(item->data(Qt::UserRole).toString()));
    }
    auto models = DBModelHelper::Fliter<DeepSeekModel>(QString("identifier in (%1)").arg(limits.join(",")));
    if (models.size() == 0)
        return;
    auto array = DBModelHelper::ToJsonArray(models);
    auto doc = QJsonDocument(array);
    auto json = doc.toJson();
    auto fileName = QFileDialog::getSaveFileName(this, "导出聊天内容", QApplication::applicationDirPath() + "/chats.json", "JSON(*.json)");
    if (!fileName.isEmpty())
    {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly))
        {
            file.write(json);
            file.close();
        }
        QMessageBox::information(this, "提示", QString("导出成功: %1").arg(fileName));
        _logger->info(QString("Export Chat Success: %1").arg(fileName));
    }
}

void DeepSeekWidget::leadChat()
{
    auto fileName = QFileDialog::getOpenFileName(this, "导入聊天内容", QApplication::applicationDirPath(), "JSON(*.json)");
    if (!fileName.isEmpty())
    {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly))
        {
            auto json = file.readAll();
            auto doc = QJsonDocument::fromJson(json);
            auto array = doc.array();
            auto models = DBModelHelper::FromJsonArray<DeepSeekModel>(array);
            for (auto model : models)
                model->identifier_set(QUuid::createUuid().toString());
            DBModelHelper::InsertModels(models);
            this->loadChat();
            QMessageBox::information(this, "提示", QString("导入成功: %1").arg(fileName));
        }
    }
}

void DeepSeekWidget::reNameChat()
{
    auto item = ui->chatListWidget->currentItem();
    if (!item)
        return;
    auto identifier = item->data(Qt::UserRole).toString();
    auto model = DBModelHelper::Fliter<DeepSeekModel>(QString("identifier = '%1'").arg(identifier)).first();
    auto chat_name = model->chat_name_get();
    auto ret = QInputDialog::getText(this, "重命名", "新名称:", QLineEdit::Normal, chat_name);
    if (ret.isEmpty())
        return;
    model->chat_name_set(ret);
    model->Update();
    this->loadChat();
}

ChatFrame *DeepSeekWidget::lastChatFrame()
{
    int count = _mainLayout->count() - 1;
    if (count < 2)
        return nullptr;
    auto widget = _mainLayout->itemAt(count - 2)->widget();
    return static_cast<ChatFrame *>(widget);
}
