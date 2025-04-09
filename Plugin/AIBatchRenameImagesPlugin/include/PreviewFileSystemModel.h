#ifndef PREVIEWFILESYSTEMMODEL_H
#define PREVIEWFILESYSTEMMODEL_H

#include <QFileSystemModel>
#include <QCache>
#include <QThreadPool>
#include <QRunnable>
#include <QPixmap>
#include <QFutureWatcher>
#include <QFuture>
#include <QtConcurrent>
#include <QStyledItemDelegate>

class CustomDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit CustomDelegate(QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;
};

class PreviewFileSystemModel : public QFileSystemModel
{
    Q_OBJECT

public:
    explicit PreviewFileSystemModel(QObject *parent = nullptr);
    ~PreviewFileSystemModel();

    // 重写基类方法
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    // 设置缩略图大小
    void setThumbnailSize(const QSize &size);
    QSize thumbnailSize() const;

    // 清除缓存
    void clearCache();

protected:
    // 当文件变更时清除缓存
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

private slots:
    void thumbnailLoaded(const QString &path, const QPixmap &pixmap);

private:
    // 生成缩略图的异步任务
    class ThumbnailTask : public QRunnable
    {
    public:
        ThumbnailTask(PreviewFileSystemModel *model, const QString &path, const QSize &size);
        void run() override;

    private:
        PreviewFileSystemModel *m_model;
        QString m_path;
        QSize m_size;
    };

    // 获取缩略图
    QPixmap getThumbnail(const QString &path) const;
    // 生成缩略图
    QPixmap generateThumbnail(const QString &path) const;

    mutable QCache<QString, QPixmap> m_thumbnailCache;
    QSize m_thumbnailSize;
    mutable QThreadPool m_threadPool;
    QMap<QString, QString> _customNames;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
};

#endif // PREVIEWFILESYSTEMMODEL_H