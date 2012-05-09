/*
 * Copyright (C) 2010-2012 Jeremy Lainé
 * Contact: http://code.google.com/p/qdjango/
 *
 * This file is part of the QDjango Library.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QDebug>
#include <QMetaProperty>
#include <QSqlDriver>
#include <QStringList>

#include "QDjango.h"
#include "QDjangoMetaModel.h"
#include "QDjangoQuerySet_p.h"

QDjangoMetaField::QDjangoMetaField()
    : autoIncrement(false),
    index(false),
    maxLength(0),
    null(false),
    unique(false)
{
}

QVariant QDjangoMetaField::toDatabase(const QVariant &value) const
{
    if (type == QVariant::String && !null && value.isNull())
        return QString("");
    else
        return value;
}

static QMap<QString, QString> parseOptions(const char *value)
{
    QMap<QString, QString> options;
    QStringList items = QString::fromUtf8(value).split(' ');
    foreach (const QString &item, items) {
        QStringList assign = item.split('=');
        if (assign.size() == 2) {
            options[assign[0].toLower()] = assign[1];
        } else {
            qWarning() << "Could not parse option" << item;
        }
    }
    return options;
}

class QDjangoMetaModelPrivate : public QSharedData
{
public:
    QList<QDjangoMetaField> localFields;
    QMap<QByteArray, QString> foreignFields;
    QByteArray primaryKey;
    QString table;
};

/*!
    Constructs a new QDjangoMetaModel by inspecting the given \a model instance.
*/
QDjangoMetaModel::QDjangoMetaModel(const QObject *model)
    : d(new QDjangoMetaModelPrivate)
{
    if (!model)
        return;

    const QMetaObject* meta = model->metaObject();
    d->table = QString(meta->className()).toLower().toLatin1();

    // parse table options
    const int optionsIndex = meta->indexOfClassInfo("__meta__");
    if (optionsIndex >= 0) {
        QMap<QString, QString> options = parseOptions(meta->classInfo(optionsIndex).value());
        QMapIterator<QString, QString> option(options);
        while (option.hasNext()) {
            option.next();
            if (option.key() == "db_table")
                d->table = option.value();
        }
    }

    const int count = meta->propertyCount();
    for(int i = QObject::staticMetaObject.propertyCount(); i < count; ++i)
    {
        QString typeName = meta->property(i).typeName();
        if (!qstrcmp(meta->property(i).name(), "pk"))
            continue;

        // parse field options
        bool autoIncrementOption = false;
        bool dbIndexOption = false;
        bool ignoreFieldOption = false;
        int maxLengthOption = 0;
        bool primaryKeyOption = false;
        bool nullOption = false;
        bool uniqueOption = false;
        const int infoIndex = meta->indexOfClassInfo(meta->property(i).name());
        if (infoIndex >= 0)
        {
            QMap<QString, QString> options = parseOptions(meta->classInfo(infoIndex).value());
            QMapIterator<QString, QString> option(options);
            while (option.hasNext()) {
                option.next();
                const QString value = option.value();
                if (option.key() == "auto_increment")
                    autoIncrementOption = (value.toLower() == "true" || value == "1");
                else if (option.key() == "db_index")
                    dbIndexOption = (value.toLower() == "true" || value == "1");
                else if (option.key() == "ignore_field")
                    ignoreFieldOption = (value.toLower() == "true" || value == "1");
                else if (option.key() == "max_length")
                    maxLengthOption = value.toInt();
                else if (option.key() == "null")
                    nullOption = (value.toLower() == "true" || value == "1");
                else if (option.key() == "primary_key")
                    primaryKeyOption = (value.toLower() == "true" || value == "1");
                else if (option.key() == "unique")
                    uniqueOption = (value.toLower() == "true" || value == "1");
            }
        }

        // ignore field
        if (ignoreFieldOption)
            continue;

        // foreign field
        if (typeName.endsWith("*"))
        {
            const QByteArray fkName = meta->property(i).name();
            const QString fkModel = typeName.left(typeName.size() - 1);
            d->foreignFields.insert(fkName, fkModel);

            QDjangoMetaField field;
            field.name = fkName + "_id";
            // FIXME : the key is not necessarily an INTEGER field, we should
            // probably perform a lookup on the foreign model, but are we sure
            // it is already registered?
            field.type = QVariant::Int;
            field.foreignModel = fkModel;
            field.index = true;
            d->localFields << field;
            continue;
        }

        // local field
        QDjangoMetaField field;
        field.index = dbIndexOption;
        field.name = meta->property(i).name();
        field.type = meta->property(i).type();
        field.maxLength = maxLengthOption;
        field.null = nullOption;
        if (primaryKeyOption) {
            field.autoIncrement = autoIncrementOption;
            field.unique = true;
            d->primaryKey = field.name;
        } else if (uniqueOption) {
            field.unique = true;
        }

        d->localFields << field;
    }

    // automatic primary key
    if (d->primaryKey.isEmpty()) {
        QDjangoMetaField field;
        field.name = "id";
        field.type = QVariant::Int;
        field.autoIncrement = true;
        field.unique = true;
        d->localFields.prepend(field);
        d->primaryKey = field.name;
    }
 
}

/*!
    Constructs a copy of \a other.
*/
QDjangoMetaModel::QDjangoMetaModel(const QDjangoMetaModel &other)
    : d(other.d)
{
}

/*!
    Destroys the meta model.
*/
QDjangoMetaModel::~QDjangoMetaModel()
{
}

/*!
    Assigns \a other to this meta model.
*/
QDjangoMetaModel& QDjangoMetaModel::operator=(const QDjangoMetaModel& other)
{
    d = other.d;
    return *this;
}

/*!
    Creates the database table for this QDjangoMetaModel.
*/
bool QDjangoMetaModel::createTable() const
{
    QSqlDatabase db = QDjango::database();
    QSqlDriver *driver = db.driver();
    const QString driverName = db.driverName();

    QStringList propSql;
    const QString quotedTable = db.driver()->escapeIdentifier(d->table, QSqlDriver::TableName);
    foreach (const QDjangoMetaField &field, d->localFields)
    {
        QString fieldSql = driver->escapeIdentifier(field.name, QSqlDriver::FieldName);
        if (field.type == QVariant::Bool)
            fieldSql += " BOOLEAN";
        else if (field.type == QVariant::ByteArray)
        {
            if (driverName == QLatin1String("QPSQL"))
                fieldSql += " BYTEA";
            else {
                fieldSql += " BLOB";
                if (field.maxLength > 0)
                    fieldSql += QString("(%1)").arg(field.maxLength);
            }
        }
        else if (field.type == QVariant::Date)
            fieldSql += " DATE";
        else if (field.type == QVariant::DateTime)
            if (driverName == QLatin1String("QPSQL"))
                fieldSql += " TIMESTAMP";
            else
                fieldSql += " DATETIME";
        else if (field.type == QVariant::Double)
            fieldSql += " REAL";
        else if (field.type == QVariant::Int)
            fieldSql += " INTEGER";
        else if (field.type == QVariant::LongLong)
            fieldSql += " BIGINT";
        else if (field.type == QVariant::String)
        {
            if (field.maxLength > 0)
                fieldSql += QString(" VARCHAR(%1)").arg(field.maxLength);
            else
                fieldSql += " TEXT";
        }
        else if (field.type == QVariant::Time)
            fieldSql += " TIME";
        else {
            qWarning() << "Unhandled type" << field.type << "for property" << field.name;
            continue;
        }

        if (!field.null)
            fieldSql += " NOT NULL";

        // primary key
        if (field.name == d->primaryKey)
            fieldSql += " PRIMARY KEY";

        // auto-increment is backend specific
        if (field.autoIncrement) {
            if (driverName == QLatin1String("QSQLITE") ||
                driverName == QLatin1String("QSQLITE2"))
                fieldSql += QLatin1String(" AUTOINCREMENT");
            else if (driverName == QLatin1String("QMYSQL"))
                fieldSql += QLatin1String(" AUTO_INCREMENT");
            else if (driverName == QLatin1String("QPSQL"))
                fieldSql = driver->escapeIdentifier(field.name, QSqlDriver::FieldName) + " SERIAL PRIMARY KEY";
        }

        // foreign key
        if (!field.foreignModel.isEmpty())
        {
            const QDjangoMetaModel foreignMeta = QDjango::metaModel(field.foreignModel);
            fieldSql += QString(" REFERENCES %1 (%2)").arg(
                driver->escapeIdentifier(foreignMeta.d->table, QSqlDriver::TableName),
                driver->escapeIdentifier(foreignMeta.d->primaryKey, QSqlDriver::FieldName));
        }
        propSql << fieldSql;
    }

    // create table
    QDjangoQuery createQuery(db);
    if (!createQuery.exec(QString("CREATE TABLE %1 (%2)").arg(
            quotedTable,
            propSql.join(", "))))
        return false;

    // create indices
    foreach (const QDjangoMetaField &field, d->localFields) {
        if (field.index && !field.unique) {
            const QString indexName = d->table + "_" + QString::fromLatin1(field.name);
            if (!createQuery.exec(QString("CREATE INDEX %1 ON %2 (%3)").arg(
                // FIXME : how should we escape an index name?
                driver->escapeIdentifier(indexName, QSqlDriver::FieldName),
                quotedTable,
                driver->escapeIdentifier(field.name, QSqlDriver::FieldName))))
                return false;
        }
    }

    return true;
}

/*!
    Drops the database table for this QDjangoMetaModel.
*/
bool QDjangoMetaModel::dropTable() const
{
    QSqlDatabase db = QDjango::database();

    QDjangoQuery query(db);
    return query.exec(QString("DROP TABLE %1").arg(
        db.driver()->escapeIdentifier(d->table, QSqlDriver::TableName)));
}

/*!
    Retrieves the QDjangoModel pointed to by the given foreign-key.

    \param model
    \param name
*/
QObject *QDjangoMetaModel::foreignKey(const QObject *model, const char *name) const
{
    const QByteArray prop(name);
    QObject *foreign = model->property(prop + "_ptr").value<QObject*>();
    if (!foreign)
        return 0;

    // if the foreign object was not loaded yet, do it now
    const QString foreignClass = d->foreignFields[prop];
    const QDjangoMetaModel foreignMeta = QDjango::metaModel(foreignClass);
    const QVariant foreignPk = model->property(prop + "_id");
    if (foreign->property(foreignMeta.primaryKey()) != foreignPk)
    {
        QDjangoQuerySetPrivate qs(foreignClass);
        qs.addFilter(QDjangoWhere("pk", QDjangoWhere::Equals, foreignPk));
        qs.sqlFetch();
        if (qs.properties.size() != 1 || !qs.sqlLoad(foreign, 0))
            return 0;
    }
    return foreign;
}

/*!
    Sets the QDjangoModel pointed to by the given foreign-key.

    \param model
    \param name
    \param value

    \note The \c model will take ownership of the given \c value.
*/
void QDjangoMetaModel::setForeignKey(QObject *model, const char *name, QObject *value) const
{
    const QByteArray prop(name);
    QObject *old = model->property(prop + "_ptr").value<QObject*>();
    if (old == value)
        return;
    if (old)
        delete old;

    // store the new pointer and update the foreign key
    model->setProperty(prop + "_ptr", qVariantFromValue(value));
    if (value)
    {
        const QDjangoMetaModel foreignMeta = QDjango::metaModel(d->foreignFields[prop]);
        model->setProperty(prop + "_id", value->property(foreignMeta.primaryKey()));
        value->setParent(model);
    } else {
        model->setProperty(prop + "_id", QVariant());
    }
}

/*!
    Loads the given properties into a \a model instance.
*/
void QDjangoMetaModel::load(QObject *model, const QVariantList &properties, int &pos) const
{
    // process local fields
    foreach (const QDjangoMetaField &field, d->localFields)
        model->setProperty(field.name, properties.at(pos++));

    // process foreign fields
    if (pos >= properties.size())
        return;
    foreach (const QByteArray &fkName, d->foreignFields.keys())
    {
        QObject *object = model->property(fkName + "_ptr").value<QObject*>();
        if (object)
        {
            const QDjangoMetaModel foreignMeta = QDjango::metaModel(d->foreignFields[fkName]);
            foreignMeta.load(object, properties, pos);
        }
    }
}

/*!
    Returns the foreign field mapping.
*/
QMap<QByteArray, QString> QDjangoMetaModel::foreignFields() const
{
    return d->foreignFields;
}

/*!
    Returns the list of local fields.
*/
QList<QDjangoMetaField> QDjangoMetaModel::localFields() const
{
    return d->localFields;
}

/*!
    Returns the name of the primary key for the current QDjangoMetaModel.
*/
QByteArray QDjangoMetaModel::primaryKey() const
{
    return d->primaryKey;
}

/*!
    Returns the name of the database table.
*/
QString QDjangoMetaModel::table() const
{
    return d->table;
}

/*!
    Removes the given \a model instance from the database.
*/
bool QDjangoMetaModel::remove(QObject *model) const
{
    const QVariant pk = model->property(d->primaryKey);
    QDjangoQuerySetPrivate qs(model->metaObject()->className());
    qs.addFilter(QDjangoWhere("pk", QDjangoWhere::Equals, pk));
    return qs.sqlDelete();
}

/*!
    Saves the given \a model instance to the database.

    \return true if saving succeeded, false otherwise
*/
bool QDjangoMetaModel::save(QObject *model) const
{
    QSqlDatabase db = QDjango::database();
    QSqlDriver *driver = db.driver();

    // find primary key
    QDjangoMetaField primaryKey;
    foreach (const QDjangoMetaField &field, d->localFields) {
        if (field.name == d->primaryKey) {
            primaryKey = field;
            break;
        }
    }

    const QString quotedTable = db.driver()->escapeIdentifier(d->table, QSqlDriver::TableName);
    const QVariant pk = model->property(d->primaryKey);
    if (!pk.isNull() && !(primaryKey.type == QVariant::Int && !pk.toInt()))
    {
        QDjangoQuery query(db);
        query.prepare(QString("SELECT 1 AS a FROM %1 WHERE %2 = ?").arg(
                      quotedTable,
                      driver->escapeIdentifier(d->primaryKey, QSqlDriver::FieldName)));
        query.addBindValue(pk);
        if (query.exec() && query.next())
        {
            // prepare data
            QVariantMap fields;
            foreach (const QDjangoMetaField &field, d->localFields) {
                if (field.name != d->primaryKey) {
                    const QVariant value = model->property(field.name);
                    fields.insert(field.name, field.toDatabase(value));
                }
            }

            // perform UPDATE
            QDjangoQuerySetPrivate qs(model->metaObject()->className());
            qs.addFilter(QDjangoWhere("pk", QDjangoWhere::Equals, pk));
            return qs.sqlUpdate(fields) != -1;
        }
    }

    // prepare data
    QVariantMap fields;
    foreach (const QDjangoMetaField &field, d->localFields) {
        if (!field.autoIncrement) {
            const QVariant value = model->property(field.name);
            fields.insert(field.name, field.toDatabase(value));
        }
    }

    // perform INSERT
    QStringList fieldColumns;
    QStringList fieldHolders;
    foreach (const QString &name, fields.keys()) {
        fieldColumns << driver->escapeIdentifier(name, QSqlDriver::FieldName);
        fieldHolders << "?";
    }

    QDjangoQuery query(db);
    query.prepare(QString("INSERT INTO %1 (%2) VALUES(%3)").arg(
                  quotedTable,
                  fieldColumns.join(", "), fieldHolders.join(", ")));
    foreach (const QString &name, fields.keys())
        query.addBindValue(fields.value(name));
    const bool ret = query.exec();

    // fetch autoincrement pk
    if (primaryKey.autoIncrement) {
        QVariant insertId;
        if (db.driverName() == "QPSQL") {
            QDjangoQuery query(db);
            const QString seqName = driver->escapeIdentifier(d->table + "_" + d->primaryKey + "_seq", QSqlDriver::FieldName);
            if (!query.exec("SELECT CURRVAL('" + seqName + "')") || !query.next())
                return false;
            insertId = query.value(0);
        } else {
            insertId = query.lastInsertId();
        }
        model->setProperty(d->primaryKey, insertId);
    }
    return ret;
}

