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
        auto valueString = DBModelHelper::valueToString(value, prop.type());
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
        auto valueString = DBModelHelper::valueToString(value, obj->property(i).type());
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

void DBModel::Load()
{
    if (this->id_get() == 0)
        return;
    LSqlExecutor excuteSql(QApplication::applicationDirPath() + "/config.db");
    auto obj = this->metaObject();
    QString tableName = obj->className();
    QString sql = QString("SELECT * FROM %1 WHERE id = %2").arg(tableName).arg(this->id_get());
    auto rows = excuteSql.executeQuery(sql);
    if (rows.size() == 0)
        return;
    auto row = rows.first();

    for (int i = 0; i < obj->propertyCount(); i++)
    {
        auto prop = obj->property(i);
        auto field = prop.name();
        auto value = row[field];
        prop.write(this, value);
    }
}

QString DBModelHelper::valueToString(const QVariant &value, QVariant::Type type)
{
    switch (type)
    {
    case QVariant::Int:
        return QString::number(value.toInt());
    case QVariant::Double:
        return QString::number(value.toDouble());
    case QVariant::String:
        return DBModelHelper::addQuotes(value.toString());
    case QVariant::ByteArray:
    {
        QString Six = value.toByteArray().toHex();
        return "X" + DBModelHelper::addQuotes(Six);
    }
    default:
        return "NULL";
    }
}

QString DBModelHelper::addQuotes(const QString &value)
{
    return "'" + value + "'";
}
