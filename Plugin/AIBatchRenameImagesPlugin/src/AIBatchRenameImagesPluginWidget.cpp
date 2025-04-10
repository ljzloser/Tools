#include "AIBatchRenameImagesPluginWidget.h"
#include <QMessageBox>
#include <QImage>
#include <QIcon>
#include <qdesktopservices.h>
#include <QFileSystemModel>
#include "PreviewFileSystemModel.h"

AIBatchRenameImagesWidget::AIBatchRenameImagesWidget(Logger *logger, TConfig *config, QWidget *parent)
    : QWidget(parent), ui(new Ui::AIBatchRenameImagesPluginWidget()), _config(config), _logger(logger)
{
    // 初始化AI为空
    _imageAI = nullptr;
    auto aiModel = _config->read("AIModel").value.value<ComboxData>().currentText();
    auto token = _config->read("AIToken").value.toString();
    this->selectAIModel(aiModel, token);
    ui->setupUi(this);
    this->initUi();
    this->initConnect();
}

AIBatchRenameImagesWidget::~AIBatchRenameImagesWidget()
{
    delete ui;
}
/**
 * @brief 初始化UI界面最好在这里完成
 */
void AIBatchRenameImagesWidget::initUi()
{
    auto info = ui->selectDirEdit->info();
    info.mode = QFileDialog::Directory; // 设置为选择文件夹模式
    info.title = "选择文件夹";          // 设置 QFileDialog 的标题为 "选择文件夹"
    ui->selectDirEdit->setInfo(info);   // 设置 QFileDialog 的信息为 info，即设置为选择文件夹模式
    ui->runButton->setCheckable(true);
    ui->dateEdit->setDate(QDate::currentDate());
}
/**
 * @brief 初始化各种信号和槽最好在这里完成
 */
void AIBatchRenameImagesWidget::initConnect()
{
    connect(ui->selectDirEdit, &LFileLineEdit::fileSelected, this, &AIBatchRenameImagesWidget::loadImageDirectory);
    connect(ui->dirTreeView, &QTreeView::doubleClicked, this, &AIBatchRenameImagesWidget::loadImages);
    connect(ui->picTableView, &QTableView::doubleClicked, this, &AIBatchRenameImagesWidget::openImage);
    connect(ui->runButton, &QPushButton::clicked, this, &AIBatchRenameImagesWidget::runButtonClicked);
    connect(_imageAI, &BaseImageAI::imageInfoChangedSignal, this, &AIBatchRenameImagesWidget::imageInfoChanged);
}

void AIBatchRenameImagesWidget::loadImages(const QModelIndex &index)
{
    QString path = qobject_cast<QFileSystemModel *>(ui->dirTreeView->model())->filePath(index);
    _logger->info("加载图片目录：" + path);
    auto model = new PreviewFileSystemModel(this);
    model->setRootPath(path);
    model->setFilter(QDir::Files | QDir::NoDotAndDotDot);
    model->setNameFilters({"*.jpg", "*.jpeg", "*.png", "*.bmp", "*.gif", "*.ico", "*.icns", "*.jfif", "*.jpe", "*.jpeg", "*.j2c", "*.j2k", "*.jp2", "*.jpc", "*.jpf", "*.jpx", "*.apng", "*.bpg", "*.bw", "*.rgb", "*.rgba", "*.sgi", "*.tiff", "*.webp"});
    model->setNameFilterDisables(false);
    ui->picTableView->setModel(model);
    ui->picTableView->setRootIndex(model->index(path));
    // 设置行高为 80
    ui->picTableView->verticalHeader()->setDefaultSectionSize(80);
    // 设置第一列的宽度为 80
    ui->picTableView->setColumnWidth(0, 80);
    // 设置第二列的宽度为 120
    ui->picTableView->setColumnWidth(1, 120);
    // 隐藏第四列和第五列
    ui->picTableView->setColumnHidden(3, true);
    ui->picTableView->setColumnHidden(4, true);
    // 最后一列设置为自适应宽度
    ui->picTableView->horizontalHeader()->setStretchLastSection(true);
    ui->picTableView->setItemDelegateForColumn(5, new CustomDelegate(this));
    ui->currentDirLabel->setText(path);
}

void AIBatchRenameImagesWidget::openImage(const QModelIndex &index)
{
    if (index.column() == 0)
    {
        QString path = qobject_cast<QFileSystemModel *>(ui->picTableView->model())->filePath(index);
        QDesktopServices::openUrl(QUrl::fromLocalFile(path));
    }
}

void AIBatchRenameImagesWidget::runButtonClicked(bool checked)
{
    if (checked)
    {
        if (_imageAI == nullptr)
        {
            throw std::runtime_error("AI模型为空");
        }
        QList<ImageInfo> imageList;
        auto date = ui->dateEdit->date().toString("yyyy-MM-dd");
        auto place = ui->placeEdit->text();

        for (int i = 0; i < ui->picTableView->model()->rowCount(ui->picTableView->rootIndex()); i++)
        {
            auto index = ui->picTableView->model()->index(i, 0, ui->picTableView->rootIndex());
            auto path = qobject_cast<QFileSystemModel *>(ui->picTableView->model())->filePath(index);
            auto imageInfo = ImageInfo(path);
            // 刷新界面使得不卡
            QApplication::processEvents();
            imageInfo.setPrompt(QString("[日期]:%1\n[地点]:%2").arg(date, place));
            imageList.append(imageInfo);
        }
        _imageAI->reSet(imageList);
        ui->runButton->setText("停止");
        ui->progressBar->setRange(0, imageList.size());
        _imageAI->start();
    }
    else
    {
        ui->runButton->setText("运行");
        _imageAI->stop();
        ui->progressBar->setRange(0, 0);
    }
}

void AIBatchRenameImagesWidget::imageInfoChanged(const ImageInfo &imageInfo, int progress)
{
    ui->progressBar->setValue(progress);
    ui->progressBar->setFormat(QString("处理完成：%1 / %2").arg(imageInfo.oldFileName(), imageInfo.newFileName()));
    ui->progressBar->update();
    auto model = qobject_cast<PreviewFileSystemModel *>(ui->picTableView->model());
    auto row = model->index(imageInfo.fileDir() + "/" + imageInfo.oldFileName());
    QModelIndex index = model->index(row.row(), 5, row.parent());
    model->setData(index, imageInfo.newFileName(), Qt::EditRole);
    if (!imageInfo.message().isEmpty())
    {
        _logger->error(QString("AI处理失败：%1").arg(imageInfo.message()));
    }
    if (progress == ui->progressBar->maximum())
    {
        QMessageBox::information(this, "提示", "处理完成");
        ui->runButton->setText("运行");
    }
}

void AIBatchRenameImagesWidget::selectAIModel(QString &model, QString &token)
{
    if (model == "qwen-vl-max-latest")
    {
        _imageAI = new TongYi_VL_Max();
        _imageAI->setModel(model);
        _imageAI->setToken(token);
    }
}

void AIBatchRenameImagesWidget::loadImageDirectory(const QStringList &files, const QString &filter)
{
    if (files.isEmpty())
        return;
    QString directory = files.first(); // 获取第一个文件的路径，即文件夹路径
    _logger->info("加载根目录：" + directory);
    auto model = new QFileSystemModel(this);                // 创建一个 QFileSystemModel 对象，用于显示文件夹中的文件和文件夹
    model->setRootPath(directory);                          // 设置 QFileSystemModel 的根路径为文件夹路径
    model->setFilter(QDir::AllDirs | QDir::NoDotAndDotDot); // 设置 QFileSystemModel 显示文件夹
    ui->dirTreeView->setModel(model);
    ui->dirTreeView->setRootIndex(model->index(directory)); // 将 QFileSystemModel 设置为 QTreeView 的模型      // 将 QFileSystemModel 设置为 QTreeView 的模型
    // 只显示名称
    ui->dirTreeView->setColumnHidden(1, true);
    ui->dirTreeView->setColumnHidden(2, true);
    ui->dirTreeView->setColumnHidden(3, true);
}
