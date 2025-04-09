#include "PreviewFileSystemModel.h"
#include <QImageReader>
#include <QPainter>
#include <QFileInfo>
#include <QDebug>
#include <QLineEdit>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

CustomDelegate::CustomDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
}

QWidget *CustomDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const
{
    Q_UNUSED(option);
    QLineEdit *editor = new QLineEdit(parent);
    editor->setStyleSheet("background: white; color: black;");
    QRegularExpression regExp("^[^\\\\/:*?\"<>|]*$"); // 排除Windows非法字符
    QRegularExpressionValidator *validator = new QRegularExpressionValidator(regExp, editor);
    editor->setValidator(validator);
    return editor;
}

void CustomDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QLineEdit *lineEdit = static_cast<QLineEdit *>(editor);
    lineEdit->setText(index.data(Qt::EditRole).toString());
}

void CustomDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                  const QModelIndex &index) const
{
    QLineEdit *lineEdit = static_cast<QLineEdit *>(editor);
    model->setData(index, lineEdit->text(), Qt::EditRole);
}

PreviewFileSystemModel::PreviewFileSystemModel(QObject *parent)
    : QFileSystemModel(parent),
      m_thumbnailSize(64, 64),
      m_threadPool()
{
    m_threadPool.setMaxThreadCount(QThread::idealThreadCount());
    m_thumbnailCache.setMaxCost(50 * 1024 * 1024); // 50MB 缓存
}

PreviewFileSystemModel::~PreviewFileSystemModel()
{
    m_threadPool.waitForDone();
}

int PreviewFileSystemModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return QFileSystemModel::columnCount() + 2;
}

QVariant PreviewFileSystemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    // 第一列是预览列
    if (index.column() == 0)
    {
        if (role == Qt::DecorationRole)
        {
            QString path = filePath(index);
            QFileInfo info(path);

            // 如果是目录，返回默认图标
            if (info.isDir())
            {
                return QFileSystemModel::data(index, role);
            }

            // 从缓存获取缩略图
            QPixmap thumbnail = getThumbnail(path);

            // 如果缓存中没有，启动异步加载
            if (thumbnail.isNull())
            {
                // 使用线程池异步加载
                ThumbnailTask *task = new ThumbnailTask(const_cast<PreviewFileSystemModel *>(this), path, m_thumbnailSize);
                const_cast<QThreadPool &>(m_threadPool).start(task); // 使用 const_cast 移除 const 限定

                // 返回占位符或默认图标
                return QFileSystemModel::data(index, role);
            }

            return thumbnail;
        }
        else if (role == Qt::SizeHintRole)
        {
            return m_thumbnailSize;
        }
    }
    else if (index.column() == 5)
    {
        if (role == Qt::DisplayRole || role == Qt::EditRole)
        {
            QString path = filePath(index);
            return _customNames[path];
        }
    }
    // 其他列和角色调用基类实现
    return QFileSystemModel::data(createIndex(index.row(), index.column() - 1, index.internalPointer()), role);
}

QVariant PreviewFileSystemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        if (section == 0)
        {
            return tr("Preview");
        }
        else if (section == 5)
        {
            return tr("New Name");
        }
        return QFileSystemModel::headerData(section - 1, orientation, role);
    }
    return QVariant();
}

void PreviewFileSystemModel::setThumbnailSize(const QSize &size)
{
    if (m_thumbnailSize != size)
    {
        m_thumbnailSize = size;
        clearCache();
    }
}

QSize PreviewFileSystemModel::thumbnailSize() const
{
    return m_thumbnailSize;
}

void PreviewFileSystemModel::clearCache()
{
    m_thumbnailCache.clear();
}

bool PreviewFileSystemModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    // 如果文件被修改，清除缓存
    if (role == Qt::EditRole && index.column() == 0)
    {
        QString path = filePath(index);
        m_thumbnailCache.remove(path);
    }
    else if (role == Qt::EditRole && index.column() == 5)
    {
        QString path = filePath(createIndex(index.row(), 0, index.internalPointer()));
        _customNames[path] = value.toString();

        // 通知视图数据已更改
        emit dataChanged(index, index, {role});
        return true;
    }
    return QFileSystemModel::setData(index, value, role);
}

void PreviewFileSystemModel::thumbnailLoaded(const QString &path, const QPixmap &pixmap)
{
    if (!pixmap.isNull())
    {
        m_thumbnailCache.insert(path, new QPixmap(pixmap),
                                (pixmap.width() * pixmap.height() * pixmap.depth() / 8) / 1024);

        // 找到所有显示此路径的索引并更新
        QModelIndexList indexes = match(index(0, 0), Qt::DisplayRole, path, -1, Qt::MatchFixedString | Qt::MatchRecursive);
        for (const QModelIndex &idx : indexes)
        {
            emit dataChanged(idx, idx, {Qt::DecorationRole});
        }
    }
}

QPixmap PreviewFileSystemModel::getThumbnail(const QString &path) const
{
    QPixmap *cached = m_thumbnailCache.object(path);
    return cached ? *cached : QPixmap();
}

QPixmap PreviewFileSystemModel::generateThumbnail(const QString &path) const
{
    QImageReader reader(path);
    if (!reader.canRead())
    {
        return QPixmap();
    }

    // 设置缩略图尺寸
    reader.setScaledSize(m_thumbnailSize);
    QImage image = reader.read();

    if (image.isNull())
    {
        return QPixmap();
    }

    // 转换为Pixmap并保持纵横比
    QPixmap pixmap = QPixmap::fromImage(image);
    if (pixmap.width() > m_thumbnailSize.width() || pixmap.height() > m_thumbnailSize.height())
    {
        pixmap = pixmap.scaled(m_thumbnailSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    return pixmap;
}

PreviewFileSystemModel::ThumbnailTask::ThumbnailTask(PreviewFileSystemModel *model, const QString &path, const QSize &size)
    : m_model(model), m_path(path), m_size(size)
{
    setAutoDelete(true);
}

void PreviewFileSystemModel::ThumbnailTask::run()
{
    QPixmap thumbnail = m_model->generateThumbnail(m_path);
    QMetaObject::invokeMethod(m_model, "thumbnailLoaded", Qt::QueuedConnection,
                              Q_ARG(QString, m_path), Q_ARG(QPixmap, thumbnail));
}
Qt::ItemFlags PreviewFileSystemModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QFileSystemModel::flags(index);

    // 使最后一列可编辑
    if (index.column() == 5)
    {
        flags |= Qt::ItemIsEditable;
    }

    return flags;
}
