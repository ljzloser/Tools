#pragma once
#include "AIBatchRenameImagesPlugin_global.h"
#include "ui_AIBatchRenameImagesPluginWidget.h"
#include <config.h>
#include <AbstractPlugin.h>
#include "AIBatchRenameImagesModel.h"
#include "TongYi_VL_Max.h"

class AIBatchRenameImagesPlugin_EXPORT AIBatchRenameImagesWidget : public QWidget
{

    Q_OBJECT
public:
    AIBatchRenameImagesWidget(Logger *logger, TConfig *config, QWidget *parent = nullptr);
    ~AIBatchRenameImagesWidget();

private:
    void initUi();
    void initConnect();
    Ui::AIBatchRenameImagesPluginWidget *ui;
    TConfig *_config;
    Logger *_logger;
    BaseImageAI *_imageAI;
private slots:
    void loadImageDirectory(const QStringList &files, const QString &filter);
    void loadImages(const QModelIndex &index);
    void openImage(const QModelIndex &index);
    void runButtonClicked(bool checked);
    void imageInfoChanged(const ImageInfo &imageInfo, int progress);
    void selectAIModel(QString &model, QString &token);
};