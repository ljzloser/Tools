#pragma once
#include <QObject>
#include <QMap>
#include <QVariant>
#include <QMetaObject>
#include <QMetaProperty>
#include <LCore>
#include "Utility_global.h"
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

using DBModelList = QList<DBModel *>;
class UTILITY_EXPORT DBModel : public QObject
{
    friend class DBModeHelper;
    Q_OBJECT
public:
    DBModel();
    ~DBModel();
    void Insert();
    void Update();
    void Delete();
    int id_get() const { return id; }

private:
    int id = 0;
    void id_set(int value) { id = value; }
};

class UTILITY_EXPORT DBModeHelper
{
public:
    static void UpdateModels(DBModelList models);
    static void InsertModels(DBModelList models);
    static void DeleteModels(DBModelList models);
    // 模板函数
    template <typename Model>
    static QList<Model *> Fliter(QString limit)
    {
        // 判断Model 是否 是 DBModel 的子类
        static_assert(std::is_base_of<DBModel, Model>::value, "Model must be a subclass of DBModel");
        QList<Model *> models;
        LSqlExecutor excuteSql(QApplication::applicationDirPath() + "/config.db");
        QString tableName = Model().metaObject()->className();
        QString sql = QString("SELECT * FROM %1 WHERE %2").arg(tableName).arg(limit);
        auto rows = excuteSql.executeQuery(sql);
        for (auto row : rows)
        {
            Model *model = new Model();
            model->id_set(row["id"].toInt());
            for (int i = 0; i < model->metaObject()->propertyCount(); i++)
            {
                auto prop = model->metaObject()->property(i);
                auto field = prop.name();
                auto value = row[field];
                prop.write(model, value);
            }
            models.append(model);
        }
        return models;
    }
    template <typename Model>
    static void Create()
    {
        // 判断Model 是否 是 DBModel 的子类
        static_assert(std::is_base_of<DBModel, Model>::value, "Model must be a subclass of DBModel");
        LSqlExecutor excuteSql(QApplication::applicationDirPath() + "/config.db");
        auto obj = Model().metaObject();
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
            case QVariant::Date:
            case QVariant::Time:
            case QVariant::DateTime:
            case QVariant::String:
            case QVariant::Char:
            case QVariant::StringList:
            case QVariant::List:
            case QVariant::Map:
                fieldType = "TEXT";
                break;
            case QVariant::UInt:
            case QVariant::ULongLong:
            case QVariant::Int:
            case QVariant::LongLong:
                fieldType = "INTEGER";
                break;
            case QVariant::Double:
                fieldType = "REAL";
                break;
            case QVariant::Bool:
                fieldType = "INTEGER";
                break;
            case QVariant::ByteArray:
            case QVariant::BitArray:
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

    static QString valueToString(const QVariant &value, QVariant::Type type);
    static QString addQuotes(const QString &value);
};