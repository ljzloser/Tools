#pragma once
#include "DeepSeekPlugin_global.h"
#include "ui_DeepSeekPluginWidget.h"
#include <config.h>
#include <AbstractPlugin.h>
#include <DeepSeek.h>
#include "ChatFrame.h"
#include <QLayout>
#include <QEvent>
#include <QKeyEvent>
#include <QWidget>
#include <LCore>

class DeepSeekPlugin_EXPORT DeepSeekWidget : public QWidget
{

    Q_OBJECT
public:
    DeepSeekWidget(Logger *logger, TConfig *config, QWidget *parent = nullptr);
    ~DeepSeekWidget();

public slots:
    void setParmas(QString key, QVariant value);

private:
    Ui::DeepSeekPluginWidget *ui;
    TConfig *_config;
    Logger *_logger;
    QString _identifier;
    QString _name;
    QWidget *_mainWidget = new QWidget(this);
    QVBoxLayout *_mainLayout = new QVBoxLayout(_mainWidget);
    DeepSeek *deepSeek = nullptr;
    QTimer *_timer = new QTimer(this);

private:
    void initUi();
    void initConnect();
    virtual void keyPressEvent(QKeyEvent *event) override;
    QList<DeepSeek::Message> oldMessage();
    ChatFrame *lastChatFrame();
private slots:
    void addLastMessage(const DeepSeek::Message &message);
    void finished(QNetworkReply::NetworkError error, int httpStatusCode, const QString &errorString);
    void newChat();
    void loadChat();
    void showContextMenu(const QPoint &pos);
    void showListWidgetContextMenu(const QPoint &pos);
    void loadChatMessage(QListWidgetItem *item);
    void deleteChat();
    void rollLast();
    void updateBalance(DeepSeek::Balance balance);
    void exportChat();
    void leadChat();
};