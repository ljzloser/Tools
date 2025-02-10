#include <DBModel.h>

DBModel::DBModel(/* args */)
    : QObject()
{
}

DBModel::~DBModel()
{
}

void DBModel::Insert()
{
    if (this->id_get() != 0)
        return;
    LSqlExecutor excuteSql(QApplication::applicationDirPath() + "/config.db");
    this->id_set(excuteSql.executeScalar<int>(QString("SELECT MAX(id) FROM %1")
                                                  .arg(this->metaObject()->className()),
                                              0) +
                 1);
    auto obj = this->metaObject();
    QString tableName = obj->className();
    QStringList fields;
    QStringList values;
    for (int i = 0; i < obj->propertyCount(); i++)
    {
        auto prop = obj->property(i);
        auto field = prop.name();
        auto value = prop.read(this);
        auto valueString = DBModeHelper::valueToString(value, prop.type());
        fields.append(field);
        values.append(valueString);
    }
    QString sql = QString("INSERT INTO %1 (id, %2) VALUES (%4, %3)")
                      .arg(tableName)
                      .arg(fields.join(","))
                      .arg(values.join(","))
                      .arg(this->id_get());
    excuteSql.executeNonQuery(sql);
}

void DBModel::Update()
{
    if (this->id_get() == 0)
        return;
    LSqlExecutor excuteSql(QApplication::applicationDirPath() + "/config.db");
    auto obj = this->metaObject();
    QString tableName = obj->className();
    QStringList setStrings;
    for (int i = 0; i < obj->propertyCount(); ++i)
    {
        QString fieldName = obj->property(i).name();
        QVariant value = obj->property(i).read(this);
        auto valueString = DBModeHelper::valueToString(value, obj->property(i).type());
        setStrings.append(QString("%1 = %2").arg(fieldName).arg(valueString));
    }
    QString sql = QString("UPDATE %1 SET %2 WHERE id = %3").arg(tableName).arg(setStrings.join(",")).arg(this->id);
    excuteSql.executeNonQuery(sql);
}

void DBModel::Delete()
{
    if (this->id_get() == 0)
        return;
    LSqlExecutor excuteSql(QApplication::applicationDirPath() + "/config.db");
    auto obj = this->metaObject();
    QString tableName = obj->className();
    QString sql = QString("DELETE FROM %1 WHERE id = %2").arg(tableName).arg(this->id);
    excuteSql.executeNonQuery(sql);
}

void DBModeHelper::DeleteModels(DBModelList models)
{
    if (models.size() == 0)
        return;
    LSqlExecutor excuteSql(QApplication::applicationDirPath() + "/config.db");
    auto obj = models.first()->metaObject();
    QString tableName = obj->className();
    QStringList ids;
    for (auto model : models)
    {
        ids.append(QString::number(model->id_get()));
    }
    QString sql = QString("DELETE FROM %1 WHERE id IN (%2)").arg(tableName).arg(ids.join(","));
    excuteSql.executeNonQuery(sql);
}

void DBModeHelper::UpdateModels(DBModelList models)
{
    if (models.size() == 0)
        return;
    for (auto model : models)
    {
        model->Update();
    }
}

void DBModeHelper::InsertModels(DBModelList models)
{
    if (models.size() == 0)
        return;
    for (auto model : models)
    {
        model->Insert();
    }
}

QString DBModeHelper::valueToString(const QVariant &value, QVariant::Type type)
{
    switch (type)
    {
    case QVariant::Int:
    case QVariant::UInt:
    case QVariant::ULongLong:
    case QVariant::LongLong:
        return QString::number(value.toInt());
    case QVariant::Double:
        return QString::number(value.toDouble());
    case QVariant::Bool:
        return QString::number(value.toBool());
    case QVariant::Date:
        return DBModeHelper::addQuotes(value.toDate().toString("yyyy-MM-dd"));
    case QVariant::Time:
        return DBModeHelper::addQuotes(value.toTime().toString("hh:mm:ss"));
    case QVariant::DateTime:
        return DBModeHelper::addQuotes(value.toDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    case QVariant::String:
        return DBModeHelper::addQuotes(value.toString());
    case QVariant::ByteArray:
    {
        QString Six = value.toByteArray().toHex();
        return "X" + DBModeHelper::addQuotes(Six);
    }
    case QVariant::BitArray:
    {
        QByteArray byteArray;
        QBitArray bitArray = value.toBitArray();
        // 将QBitArray的每8个比特转为一个字节并添加到QByteArray中
        for (int i = 0; i < bitArray.size(); i += 8)
        {
            uchar byte = 0;

            // 每8个位构成一个字节
            for (int j = 0; j < 8 && i + j < bitArray.size(); ++j)
            {
                byte |= (bitArray[i + j] << (7 - j)); // 从左到右设置比特
            }

            byteArray.append(byte); // 将字节加入QByteArray
        }
        QString Six = byteArray.toHex();
        return "X" + DBModeHelper::addQuotes(Six);
    }
    case QVariant::Char:
        return DBModeHelper::addQuotes(value.toString());
    case QVariant::StringList:
        return DBModeHelper::addQuotes(value.toStringList().join(","));
    case QVariant::List:
        return DBModeHelper::addQuotes(value.toStringList().join(","));
    case QVariant::Map:
    {
        auto map = value.toMap();
        auto obj = QJsonObject::fromVariantMap(map);
        auto json = QJsonDocument(obj).toJson(QJsonDocument::Compact);
        return DBModeHelper::addQuotes(json);
    }
    default:
        return "NULL";
    }
}

QString DBModeHelper::addQuotes(const QString &value)
{
    return "'" + value + "'";
}
