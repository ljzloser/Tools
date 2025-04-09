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
    auto info = ui->lineEdit->info();
    info.mode = QFileDialog::Directory; // 设置为选择文件夹模式
    info.title = "选择文件夹";          // 设置 QFileDialog 的标题为 "选择文件夹"
    ui->lineEdit->setInfo(info);        // 设置 QFileDialog 的信息为 info，即设置为选择文件夹模式
}
/**
 * @brief 初始化各种信号和槽最好在这里完成
 */
void AIBatchRenameImagesWidget::initConnect()
{
    connect(ui->lineEdit, &LFileLineEdit::fileSelected, this, &AIBatchRenameImagesWidget::loadImageDirectory);
    connect(ui->treeView, &QTreeView::doubleClicked, this, &AIBatchRenameImagesWidget::loadImages);
    connect(ui->tableView, &QTableView::doubleClicked, this, &AIBatchRenameImagesWidget::openImage);
}

void AIBatchRenameImagesWidget::loadImages(const QModelIndex &index)
{
    QString path = qobject_cast<QFileSystemModel *>(ui->treeView->model())->filePath(index);
    _logger->info("加载图片目录：" + path);
    auto model = new PreviewFileSystemModel(this);
    model->setRootPath(path);
    model->setFilter(QDir::Files | QDir::NoDotAndDotDot);
    model->setNameFilters({"*.jpg", "*.png", "*.bmp", "*.jpeg"});
    model->setNameFilterDisables(false);
    ui->tableView->setModel(model);
    ui->tableView->setRootIndex(model->index(path));
    // 设置行高为 80
    ui->tableView->verticalHeader()->setDefaultSectionSize(80);
    // 设置第一列的宽度为 80
    ui->tableView->setColumnWidth(0, 80);
    // 设置第二列的宽度为 120
    ui->tableView->setColumnWidth(1, 120);
    // 隐藏第四列和第五列
    ui->tableView->setColumnHidden(3, true);
    ui->tableView->setColumnHidden(4, true);
    // 最后一列设置为自适应宽度
    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    ui->tableView->setItemDelegateForColumn(5, new CustomDelegate(this));
}

void AIBatchRenameImagesWidget::openImage(const QModelIndex &index)
{
    if (index.column() == 0)
    {
        QString path = qobject_cast<QFileSystemModel *>(ui->tableView->model())->filePath(index);
        QDesktopServices::openUrl(QUrl::fromLocalFile(path));
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
    ui->treeView->setModel(model);
    ui->treeView->setRootIndex(model->index(directory)); // 将 QFileSystemModel 设置为 QTreeView 的模型      // 将 QFileSystemModel 设置为 QTreeView 的模型
    // 只显示名称
    ui->treeView->setColumnHidden(1, true);
    ui->treeView->setColumnHidden(2, true);
    ui->treeView->setColumnHidden(3, true);
}
