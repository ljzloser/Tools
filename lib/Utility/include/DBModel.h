#pragma once
#include <QObject>
#include <QMap>
#include <QVariant>
#include <QMetaObject>
#include <QMetaProperty>
#include <LCore>
#include "Utility_global.h"
#include <qmetatype.h>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

class UTILITY_EXPORT DBModel;

#define ADD_PROPERTY(type, fieldName)                                                               \
private:                                                                                            \
    type fieldName;                                                                                 \
                                                                                                    \
public:                                                                                             \
    type fieldName##_get() const { return fieldName; }                                              \
    void fieldName##_set(type value)                                                                \
    {                                                                                               \
        if (fieldName != value)                                                                     \
        {                                                                                           \
            fieldName = value;                                                                      \
            emit fieldName##Changed();                                                              \
        }                                                                                           \
    }                                                                                               \
    Q_PROPERTY(type fieldName READ fieldName##_get WRITE fieldName##_set NOTIFY fieldName##Changed) \
Q_SIGNALS:                                                                                          \
    void fieldName##Changed();

#define ADD_TEXT_PROPERTY(fieldName) ADD_PROPERTY(QString, fieldName)
#define ADD_INTEGER_PROPERTY(fieldName) ADD_PROPERTY(int, fieldName)
#define ADD_REAL_PROPERTY(fieldName) ADD_PROPERTY(double, fieldName)
#define ADD_BLOB_PROPERTY(fieldName) ADD_PROPERTY(QByteArray, fieldName)
#define ADD_DBModel_PROPERTY(fieldName) ""
#define ADD_DBModel_LIST_PROPERTY(fieldName) ""
using DBModelList = QList<QSharedPointer<DBModel>>;
class UTILITY_EXPORT DBModel : public QObject
{
    friend class DBModelHelper;
    Q_OBJECT
public:
public:
    DBModel();
    ~DBModel();
    /**
     * @brief 插入
     */
    void Insert();
    /**
     * @brief 更新
     */
    void Update();
    /**
     * @brief 删除
     */
    void Delete();
    /**
     * @brief 加载
     */
    void Load();
    int id_get() const { return id; }

private:
    int id = 0;
    void id_set(int value) { id = value; }
};

class UTILITY_EXPORT DBModelHelper
{
public:
    /**
     * @brief 更新模型
     * @param models
     */
    template <typename Model>
    static void UpdateModels(QList<QSharedPointer<Model>> models)
    {
        DBModelHelper::verifyModel<Model>();
        if (models.size() == 0)
            return;
        for (auto model : models)
        {
            model->Update();
        }
    }

    /**
     * @brief 插入模型
     */
    template <typename Model>
    static void InsertModels(QList<QSharedPointer<Model>> models)
    {
        DBModelHelper::verifyModel<Model>();
        if (models.size() == 0)
            return;
        for (auto model : models)
        {
            model->Insert();
        }
    }
    /**
     * @brief 删除模型
     */
    template <typename Model>
    static void DeleteModels(QList<QSharedPointer<Model>> models)
    {
        DBModelHelper::verifyModel<Model>();
        if (models.size() == 0)
            return;
        LSqlExecutor excuteSql(QApplication::applicationDirPath() + "/config.db");
        const QMetaObject *obj = models.first()->metaObject();
        QString tableName = obj->className();
        QStringList ids;
        for (auto model : models)
        {
            ids.append(QString::number(model->id_get()));
        }
        QString sql = QString("DELETE FROM %1 WHERE id IN (%2)").arg(tableName).arg(ids.join(","));
        excuteSql.executeNonQuery(sql);
    }

    /**
     * @brief 根据条件过滤模型
     * @tparam Model DBModel 的子类
     * @param limit 条件
     * @return 返回过滤后的模型
     */
    template <typename Model>
    static QList<QSharedPointer<Model>> Fliter(QString limit)
    {
        // 判断Model 是否 是 DBModel 的子类
        DBModelHelper::verifyModel<Model>();
        QList<QSharedPointer<Model>> models;
        LSqlExecutor excuteSql(QApplication::applicationDirPath() + "/config.db");
        QString tableName = Model().metaObject()->className();
        QString sql = QString("SELECT * FROM %1 WHERE %2").arg(tableName).arg(limit);
        auto rows = excuteSql.executeQuery(sql);
        for (auto row : rows)
        {
            auto model = QSharedPointer<Model>(new Model());
            model->id_set(row["id"].toInt());
            for (int i = 0; i < model->metaObject()->propertyCount(); i++)
            {
                auto prop = model->metaObject()->property(i);
                auto field = prop.name();
                auto value = row[field];
                prop.write(model.data(), value);
            }
            models.append(model);
        }
        return models;
    }
    /**
     * @brief 创建表
     * @tparam Model DBModel 的子类
     */
    template <typename Model>
    static void Create()
    {
        // 判断Model 是否 是 DBModel 的子类
        DBModelHelper::verifyModel<Model>();
        LSqlExecutor excuteSql(QApplication::applicationDirPath() + "/config.db");
        const QMetaObject *obj = Model().metaObject();
        QString tableName = obj->className();
        QStringList fields;
        for (int i = 0; i < obj->propertyCount(); i++)
        {
            auto prop = obj->property(i);
            auto field = prop.name();
            auto type = prop.type();
            // sqllite
            QString fieldType;
            switch (type)
            {
            case QVariant::String:
                fieldType = "TEXT";
                break;
            case QVariant::Int:
                fieldType = "INTEGER";
                break;
            case QVariant::Double:
                fieldType = "REAL";
                break;
            case QVariant::ByteArray:
                fieldType = "BLOB";
                break;
            default:
                fieldType = "TEXT";
                break;
            }
            fields.append(QString("%1 %2").arg(field).arg(fieldType));
        }
        QString sql = QString("CREATE TABLE IF NOT EXISTS %1 (id INTEGER PRIMARY KEY AUTOINCREMENT, %2)")
                          .arg(tableName)
                          .arg(fields.join(","));
        excuteSql.executeNonQuery(sql);
    }
    /**
     * @brief 将 QVariant 转换为字符串，用于数据库存储
     * @param value 值
     * @param type 类型
     * @return 字符串
     */
    static QString valueToString(const QVariant &value, QVariant::Type type);
    /**
     * @brief 添加引号
     * @param value
     * @return 字符串
     */
    static QString addQuotes(const QString &value);
    template <typename Model>
    static QJsonObject ToJsonObject(QSharedPointer<Model> model)
    {
        DBModelHelper::verifyModel<Model>();
        QJsonObject obj;
        const QMetaObject *metaObject = model->metaObject();
        for (int i = 0; i < metaObject->propertyCount(); i++)
        {
            auto prop = metaObject->property(i);
            auto field = prop.name();
            auto value = prop.read(model.data());
            obj[field] = QJsonValue::fromVariant(value);
        }
        return obj;
    }
    template <typename Model>
    static QSharedPointer<Model> FromJsonObject(QJsonObject obj)
    {
        DBModelHelper::verifyModel<Model>();
        QSharedPointer<Model> model(new Model());
        const QMetaObject *metaObject = model->metaObject();
        for (int i = 0; i < metaObject->propertyCount(); i++)
        {
            auto prop = metaObject->property(i);
            auto field = prop.name();
            auto value = obj[field].toVariant();
            prop.write(model.data(), value);
        }
        return model;
    }
    template <typename Model>
    static QList<QSharedPointer<Model>> FromJsonArray(QJsonArray array)
    {
        QList<QSharedPointer<Model>> models;
        for (auto obj : array)
        {
            models.append(FromJsonObject<Model>(obj.toObject()));
        }
        return models;
    }
    template <typename Model>
    static QJsonArray ToJsonArray(QList<QSharedPointer<Model>> models)
    {
        QJsonArray array;
        for (auto model : models)
        {
            array.append(ToJsonObject<Model>(model));
        }
        return array;
    }
    /**
     * @brief 设置值
     * @tparam Model DBModel 的子类
     * @param model 模型
     * @param fieldName 字段
     * @param value 值
     * @return 是否设置成功
     */
    template <typename Model>
    static bool SetValue(QSharedPointer<Model> model, const QString &fieldName, const QVariant &value)
    {
        DBModelHelper::verifyModel<Model>();
        const QMetaObject *metaObject = model->metaObject();
        // 判断字段是否存在
        auto prop = metaObject->property(metaObject->indexOfProperty(fieldName));
        if (!prop.isValid())
            return false;
        prop.write(model.data(), value);
        return true;
    }
    /**
     * @brief 获取值
     * @tparam Model DBModel 的子类
     * @param model 模型
     * @param fieldName 字段
     * @return 值
     */
    template <typename Model>
    static QVariant GetValue(QSharedPointer<Model> model, const QString &fieldName)
    {
        DBModelHelper::verifyModel<Model>();
        const QMetaObject *metaObject = model->metaObject();
        // 判断字段是否存在
        auto prop = metaObject->property(metaObject->indexOfProperty(fieldName));
        if (!prop.isValid())
            return QVariant();
        return prop.read(model.data());
    }

    /**
     * @brief 检查模型是否是DBModel的子类
     * @tparam Model DBModel 的子类
     */
    template <typename Model>
    static void verifyModel()
    {
        static_assert(std::is_base_of<DBModel, Model>::value, "Model must be a subclass of DBModel");
    }
};