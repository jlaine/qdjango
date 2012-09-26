/*
 * Copyright (C) 2010-2012 Jeremy Lainé
 * Contact: http://code.google.com/p/qdjango/
 *
 * This file is part of the QDjango Library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 */

#include <QDebug>
#include <QMetaProperty>
#include <QSqlDriver>
#include <QStringList>

#include "QDjango.h"
#include "QDjangoMetaModel.h"
#include "QDjangoQuerySet_p.h"

class QDjangoMetaFieldPrivate : public QSharedData
{
public:
    QDjangoMetaFieldPrivate();

    bool autoIncrement;
    QString db_column;
    QByteArray foreignModel;
    bool index;
    int maxLength;
    QByteArray name;
    bool null;
    QVariant::Type type;
    bool unique;
};

QDjangoMetaFieldPrivate::QDjangoMetaFieldPrivate()
    : autoIncrement(false),
    index(false),
    maxLength(0),
    null(false),
    unique(false)
{
}

/*!
    Constructs a new QDjangoMetaField.
*/
QDjangoMetaField::QDjangoMetaField()
{
    d = new QDjangoMetaFieldPrivate;
}

/*!
    Constructs a copy of \a other.
*/
QDjangoMetaField::QDjangoMetaField(const QDjangoMetaField &other)
    : d(other.d)
{
}

/*!
    Destroys the meta field.
*/
QDjangoMetaField::~QDjangoMetaField()
{
}

/*!
    Assigns \a other to this meta field.
*/
QDjangoMetaField& QDjangoMetaField::operator=(const QDjangoMetaField& other)
{
    d = other.d;
    return *this;
}

/*!
    Returns the database column for this meta field.
*/
QString QDjangoMetaField::column() const
{
    return d->db_column;
}

/*!
    Returns true if this is a valid field.
*/
bool QDjangoMetaField::isValid() const
{
    return !d->name.isEmpty();
}

/*!
    Returns name of this meta field.
*/
QString QDjangoMetaField::name() const
{
    return QString::fromLatin1(d->name);
}

/*!
    Transforms the given field value for database storage.
*/
QVariant QDjangoMetaField::toDatabase(const QVariant &value) const
{
    if (d->type == QVariant::String && !d->null && value.isNull())
        return QLatin1String("");
    else if (!d->foreignModel.isEmpty() && d->type == QVariant::Int && d->null && !value.toInt()) {
        // store 0 foreign key as NULL if the field is NULL
        return QVariant();
    } else
        return value;
}

static QMap<QString, QString> parseOptions(const char *value)
{
    QMap<QString, QString> options;
    QStringList items = QString::fromLatin1(value).split(QLatin1Char(' '));
    foreach (const QString &item, items) {
        QStringList assign = item.split(QLatin1Char('='));
        if (assign.size() == 2) {
            options[assign[0].toLower()] = assign[1];
        } else {
            qWarning() << "Could not parse option" << item;
        }
    }
    return options;
}

static bool stringToBool(const QString &value)
{
    return value.toLower() == QLatin1String("true") || value == QLatin1String("1");
}

class QDjangoMetaModelPrivate : public QSharedData
{
public:
    QList<QDjangoMetaField> localFields;
    QMap<QByteArray, QByteArray> foreignFields;
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
    d->table = QString::fromLatin1(meta->className()).toLower();

    // parse table options
    const int optionsIndex = meta->indexOfClassInfo("__meta__");
    if (optionsIndex >= 0) {
        QMap<QString, QString> options = parseOptions(meta->classInfo(optionsIndex).value());
        QMapIterator<QString, QString> option(options);
        while (option.hasNext()) {
            option.next();
            if (option.key() == QLatin1String("db_table"))
                d->table = option.value();
        }
    }

    const int count = meta->propertyCount();
    for(int i = QObject::staticMetaObject.propertyCount(); i < count; ++i)
    {
        const QString typeName = QString::fromLatin1(meta->property(i).typeName());
        if (!qstrcmp(meta->property(i).name(), "pk"))
            continue;

        // parse field options
        bool autoIncrementOption = false;
        QString dbColumnOption;
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
                const QString key = option.key();
                const QString value = option.value();
                if (key == QLatin1String("auto_increment"))
                    autoIncrementOption = stringToBool(value);
                else if (key == QLatin1String("db_column"))
                    dbColumnOption = value;
                else if (key == QLatin1String("db_index"))
                    dbIndexOption = stringToBool(value);
                else if (key == QLatin1String("ignore_field"))
                    ignoreFieldOption = stringToBool(value);
                else if (key == QLatin1String("max_length"))
                    maxLengthOption = value.toInt();
                else if (key == QLatin1String("null"))
                    nullOption = stringToBool(value);
                else if (key == QLatin1String("primary_key"))
                    primaryKeyOption = stringToBool(value);
                else if (key == QLatin1String("unique"))
                    uniqueOption = stringToBool(value);
            }
        }

        // ignore field
        if (ignoreFieldOption)
            continue;

        // foreign field
        if (typeName.endsWith(QLatin1Char('*'))) {
            const QByteArray fkName = meta->property(i).name();
            const QByteArray fkModel = typeName.left(typeName.size() - 1).toLatin1();
            d->foreignFields.insert(fkName, fkModel);

            QDjangoMetaField field;
            field.d->name = fkName + "_id";
            // FIXME : the key is not necessarily an INTEGER field, we should
            // probably perform a lookup on the foreign model, but are we sure
            // it is already registered?
            field.d->type = QVariant::Int;
            field.d->foreignModel = fkModel;
            field.d->db_column = dbColumnOption.isEmpty() ? QString::fromLatin1(field.d->name) : dbColumnOption;
            field.d->index = true;
            field.d->null = nullOption;
            d->localFields << field;
            continue;
        }

        // local field
        QDjangoMetaField field;
        field.d->name = meta->property(i).name();
        field.d->type = meta->property(i).type();
        field.d->db_column = dbColumnOption.isEmpty() ? QString::fromLatin1(field.d->name) : dbColumnOption;
        field.d->maxLength = maxLengthOption;
        field.d->null = nullOption;
        if (primaryKeyOption) {
            field.d->autoIncrement = autoIncrementOption;
            d->primaryKey = field.d->name;
        } else if (uniqueOption) {
            field.d->unique = true;
        } else if (dbIndexOption) {
            field.d->index = true;
        }

        d->localFields << field;
    }

    // automatic primary key
    if (d->primaryKey.isEmpty()) {
        QDjangoMetaField field;
        field.d->name = "id";
        field.d->type = QVariant::Int;
        field.d->db_column = QLatin1String("id");
        field.d->autoIncrement = true;
        d->localFields.prepend(field);
        d->primaryKey = field.d->name;
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
    QDjangoQuery createQuery(QDjango::database());
    foreach (const QString &sql, createTableSql()) {
        if (!createQuery.exec(sql))
            return false;
    }
    return true;
}

/*!
    Returns the SQL queries to create the database table for this
    QDjangoMetaModel.
*/
QStringList QDjangoMetaModel::createTableSql() const
{
    QSqlDatabase db = QDjango::database();
    QSqlDriver *driver = db.driver();
    const QString driverName = db.driverName();

    QStringList queries;
    QStringList propSql;
    const QString quotedTable = db.driver()->escapeIdentifier(d->table, QSqlDriver::TableName);
    foreach (const QDjangoMetaField &field, d->localFields)
    {
        QString fieldSql = driver->escapeIdentifier(field.column(), QSqlDriver::FieldName);
        switch (field.d->type) {
        case QVariant::Bool:
            if (driverName == QLatin1String("QPSQL"))
                fieldSql += QLatin1String(" boolean");
            else
                fieldSql += QLatin1String(" bool");
            break;
        case QVariant::ByteArray:
            if (driverName == QLatin1String("QPSQL"))
                fieldSql += QLatin1String(" bytea");
            else {
                fieldSql += QLatin1String(" blob");
                if (field.d->maxLength > 0)
                    fieldSql += QLatin1Char('(') + QString::number(field.d->maxLength) + QLatin1Char(')');
            }
            break;
        case QVariant::Date:
            fieldSql += QLatin1String(" date");
            break;
        case QVariant::DateTime:
            if (driverName == QLatin1String("QPSQL"))
                fieldSql += QLatin1String(" timestamp");
            else
                fieldSql += QLatin1String(" datetime");
            break;
        case QVariant::Double:
            fieldSql += QLatin1String(" real");
            break;
        case QVariant::Int:
            fieldSql += QLatin1String(" integer");
            break;
        case QVariant::LongLong:
            fieldSql += QLatin1String(" bigint");
            break;
        case QVariant::String:
            if (field.d->maxLength > 0)
                fieldSql += QLatin1String(" varchar(") + QString::number(field.d->maxLength) + QLatin1Char(')');
            else
                fieldSql += QLatin1String(" text");
            break;
        case QVariant::Time:
            fieldSql += QLatin1String(" time");
            break;
        default:
            qWarning() << "Unhandled type" << field.d->type << "for property" << field.d->name;
            continue;
        }

        if (!field.d->null)
            fieldSql += QLatin1String(" NOT NULL");
        if (field.d->unique)
            fieldSql += QLatin1String(" UNIQUE");

        // primary key
        if (field.d->name == d->primaryKey)
            fieldSql += QLatin1String(" PRIMARY KEY");

        // auto-increment is backend specific
        if (field.d->autoIncrement) {
            if (driverName == QLatin1String("QSQLITE") ||
                driverName == QLatin1String("QSQLITE2"))
                // NOTE: django does not add this option for sqlite, but there
                // is a ticket asking for it to do so:
                // https://code.djangoproject.com/ticket/10164
                fieldSql += QLatin1String(" AUTOINCREMENT");
            else if (driverName == QLatin1String("QMYSQL"))
                fieldSql += QLatin1String(" AUTO_INCREMENT");
            else if (driverName == QLatin1String("QPSQL"))
                fieldSql = driver->escapeIdentifier(field.column(), QSqlDriver::FieldName) + QLatin1String(" serial PRIMARY KEY");
        }

        // foreign key
        if (!field.d->foreignModel.isEmpty())
        {
            const QDjangoMetaModel foreignMeta = QDjango::metaModel(field.d->foreignModel);
            const QDjangoMetaField foreignField = foreignMeta.localField("pk");
            fieldSql += QString::fromLatin1(" REFERENCES %1 (%2)").arg(
                driver->escapeIdentifier(foreignMeta.d->table, QSqlDriver::TableName),
                driver->escapeIdentifier(foreignField.column(), QSqlDriver::FieldName));
        }
        propSql << fieldSql;
    }

    // create table
    queries << QString::fromLatin1("CREATE TABLE %1 (%2)").arg(
            quotedTable,
            propSql.join(QLatin1String(", ")));

    // create indices
    foreach (const QDjangoMetaField &field, d->localFields) {
        if (field.d->index) {
            const QString indexName = d->table + QLatin1Char('_') + field.column();
            queries << QString::fromLatin1("CREATE INDEX %1 ON %2 (%3)").arg(
                // FIXME : how should we escape an index name?
                driver->escapeIdentifier(indexName, QSqlDriver::FieldName),
                quotedTable,
                driver->escapeIdentifier(field.column(), QSqlDriver::FieldName));
        }
    }

    return queries;
}

/*!
    Drops the database table for this QDjangoMetaModel.
*/
bool QDjangoMetaModel::dropTable() const
{
    QSqlDatabase db = QDjango::database();

    QDjangoQuery query(db);
    return query.exec(QLatin1String("DROP TABLE ") +
        db.driver()->escapeIdentifier(d->table, QSqlDriver::TableName));
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
    const QByteArray foreignClass = d->foreignFields[prop];
    const QDjangoMetaModel foreignMeta = QDjango::metaModel(foreignClass);
    const QVariant foreignPk = model->property(prop + "_id");
    if (foreign->property(foreignMeta.primaryKey()) != foreignPk)
    {
        QDjangoQuerySetPrivate qs(foreignClass);
        qs.addFilter(QDjangoWhere(QLatin1String("pk"), QDjangoWhere::Equals, foreignPk));
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
        model->setProperty(field.d->name, properties.at(pos++));

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
QMap<QByteArray, QByteArray> QDjangoMetaModel::foreignFields() const
{
    return d->foreignFields;
}

/*!
    Return the local field with the specified \a name.
*/
QDjangoMetaField QDjangoMetaModel::localField(const char *name) const
{
    const QByteArray fieldName = strcmp(name, "pk") ? QByteArray(name) : d->primaryKey;
    foreach (const QDjangoMetaField &field, d->localFields) {
        if (field.d->name == fieldName)
            return field;
    }
    return QDjangoMetaField();
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
    qs.addFilter(QDjangoWhere(QLatin1String("pk"), QDjangoWhere::Equals, pk));
    return qs.sqlDelete();
}

/*!
    Saves the given \a model instance to the database.

    \return true if saving succeeded, false otherwise
*/
bool QDjangoMetaModel::save(QObject *model) const
{
    // find primary key
    const QDjangoMetaField primaryKey = localField("pk");
    const QVariant pk = model->property(d->primaryKey);
    if (!pk.isNull() && !(primaryKey.d->type == QVariant::Int && !pk.toInt()))
    {
        QSqlDatabase db = QDjango::database();
        QDjangoQuery query(db);
        query.prepare(QString::fromLatin1("SELECT 1 AS a FROM %1 WHERE %2 = ?").arg(
                      db.driver()->escapeIdentifier(d->table, QSqlDriver::FieldName),
                      db.driver()->escapeIdentifier(primaryKey.column(), QSqlDriver::FieldName)));
        query.addBindValue(pk);
        if (query.exec() && query.next())
        {
            // prepare data
            QVariantMap fields;
            foreach (const QDjangoMetaField &field, d->localFields) {
                if (field.d->name != d->primaryKey) {
                    const QVariant value = model->property(field.d->name);
                    fields.insert(QString::fromLatin1(field.d->name), field.toDatabase(value));
                }
            }

            // perform UPDATE
            QDjangoQuerySetPrivate qs(model->metaObject()->className());
            qs.addFilter(QDjangoWhere(QLatin1String("pk"), QDjangoWhere::Equals, pk));
            return qs.sqlUpdate(fields) != -1;
        }
    }

    // prepare data
    QVariantMap fields;
    foreach (const QDjangoMetaField &field, d->localFields) {
        if (!field.d->autoIncrement) {
            const QVariant value = model->property(field.d->name);
            fields.insert(field.name(), field.toDatabase(value));
        }
    }

    // perform INSERT
    QVariant insertId;
    QDjangoQuerySetPrivate qs(model->metaObject()->className());
    if (!qs.sqlInsert(fields, &insertId))
        return false;

    // fetch autoincrement pk
    if (primaryKey.d->autoIncrement)
        model->setProperty(d->primaryKey, insertId);
    return true;
}

