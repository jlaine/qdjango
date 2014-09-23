/*
 * Copyright (C) 2010-2014 Jeremy Lainé
 * Contact: https://github.com/jlaine/qdjango
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

// python-compatible hash

static long string_hash(const QString &s)
{
    if (s.isEmpty())
        return 0;

    const QByteArray a = s.toLatin1();
    unsigned char *p = (unsigned char *) a.constData();
    long x = *p << 7;
    for (int i = 0; i < a.size(); ++i)
        x = (1000003*x) ^ *p++;
    x ^= a.size();
    return (x == -1) ? -2 : x;
}

static long stringlist_hash(const QStringList &l)
{
    long x = 0x345678L;
    long mult = 1000003L;
    int len = l.size();
    foreach (const QString &s, l) {
        --len;
        x = (x ^ string_hash(s)) * mult;
        mult += (long)(82520L + len + len);
    }
    x += 97531L;
    return (x == -1) ? -2 : x;
}

// django-compatible digest

static QString stringlist_digest(const QStringList &l)
{
    return QString::number(labs(stringlist_hash(l)) % 4294967296UL, 16);
}

enum ForeignKeyConstraint
{
    NoAction,
    Restrict,
    Cascade,
    SetNull
};

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
    bool blank;
    ForeignKeyConstraint deleteConstraint;
};

QDjangoMetaFieldPrivate::QDjangoMetaFieldPrivate()
    : autoIncrement(false)
    , index(false)
    , maxLength(0)
    , null(false)
    , unique(false)
    , blank(false)
    , deleteConstraint(NoAction)
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
    Returns true if this field is nullable.
*/
bool QDjangoMetaField::isNullable() const
{
    return d->null;
}

/*!
    Returns true if this is a valid field.
*/
bool QDjangoMetaField::isValid() const
{
    return !d->name.isEmpty();
}

/*!
    Returns true if this field is auto incremented.
*/
bool QDjangoMetaField::isAutoIncrement() const
{
    return d->autoIncrement;
}

/*!
    Returns true if this field is unique.
*/
bool QDjangoMetaField::isUnique() const
{
    return d->unique;
}

/*!
    Returns true if this field can be empty.
*/
bool QDjangoMetaField::isBlank() const
{
    return d->blank;
}

/*!
    Returns name of this meta field.
*/
QString QDjangoMetaField::name() const
{
    return QString::fromLatin1(d->name);
}

/*!
    Returns the max length of this field
*/
int QDjangoMetaField::maxLength() const
{
    return d->maxLength;
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
    QString className;
    QList<QDjangoMetaField> localFields;
    QMap<QByteArray, QByteArray> foreignFields;
    QByteArray primaryKey;
    QString table;
    QList<QByteArray> uniqueTogether;
};

/*!
    Constructs a new QDjangoMetaModel by inspecting the given \a meta model.
*/
QDjangoMetaModel::QDjangoMetaModel(const QMetaObject *meta)
    : d(new QDjangoMetaModelPrivate)
{
    if (!meta)
        return;

    d->className = meta->className();
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
            else if (option.key() == QLatin1String("unique_together"))
                d->uniqueTogether = option.value().toLatin1().split(',');
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
        bool blankOption = false;
        ForeignKeyConstraint deleteConstraint = NoAction;
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
                else if (key == QLatin1String("blank"))
                    blankOption = stringToBool(value);
                else if (option.key() == "on_delete") {
                    if (value.toLower() == "cascade")
                        deleteConstraint = Cascade;
                    else if (value.toLower() == "set_null")
                        deleteConstraint = SetNull;
                    else if (value.toLower() == "restrict")
                        deleteConstraint = Restrict;
                }
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
            field.d->deleteConstraint = deleteConstraint;
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
        } else if (blankOption) {
            field.d->blank = true;
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

QString QDjangoMetaModel::className() const
{
    return d->className;
}

/*!
    Determine whether this is a valid model, or just default constructed
 */
bool QDjangoMetaModel::isValid() const
{
    return !d->table.isNull();
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

    \return true if the table was created, false otherwise.
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
    QDjangoDatabase::DatabaseType databaseType = QDjangoDatabase::databaseType(db);

    QStringList queries;
    QStringList propSql;
    QStringList constraintSql;
    const QString quotedTable = db.driver()->escapeIdentifier(d->table, QSqlDriver::TableName);
    foreach (const QDjangoMetaField &field, d->localFields)
    {
        QString fieldSql = driver->escapeIdentifier(field.column(), QSqlDriver::FieldName);
        switch (field.d->type) {
        case QVariant::Bool:
            if (databaseType == QDjangoDatabase::PostgreSQL)
                fieldSql += QLatin1String(" boolean");
            else if (databaseType == QDjangoDatabase::MSSqlServer)
                fieldSql += QLatin1String(" bit");
            else
                fieldSql += QLatin1String(" bool");
            break;
        case QVariant::ByteArray:
            if (databaseType == QDjangoDatabase::PostgreSQL) {
                fieldSql += QLatin1String(" bytea");
            } else if (databaseType == QDjangoDatabase::MSSqlServer) {
                fieldSql += QLatin1String(" varbinary");
                if (field.d->maxLength > 0)
                    fieldSql += QLatin1Char('(') + QString::number(field.d->maxLength) + QLatin1Char(')');
                else
                    fieldSql += QLatin1String("(max)");
            } else {
                fieldSql += QLatin1String(" blob");
                if (field.d->maxLength > 0)
                    fieldSql += QLatin1Char('(') + QString::number(field.d->maxLength) + QLatin1Char(')');
            }
            break;
        case QVariant::Date:
            fieldSql += QLatin1String(" date");
            break;
        case QVariant::DateTime:
            if (databaseType == QDjangoDatabase::PostgreSQL)
                fieldSql += QLatin1String(" timestamp");
            else
                fieldSql += QLatin1String(" datetime");
            break;
        case QVariant::Double:
            fieldSql += QLatin1String(" real");
            break;
        case QVariant::Int:
            if (databaseType == QDjangoDatabase::MSSqlServer)
                fieldSql += QLatin1String(" int");
            else
                fieldSql += QLatin1String(" integer");
            break;
        case QVariant::LongLong:
            fieldSql += QLatin1String(" bigint");
            break;
        case QVariant::String:
            if (field.d->maxLength > 0) {
                if (databaseType == QDjangoDatabase::MSSqlServer)
                    fieldSql += QLatin1String(" nvarchar(") + QString::number(field.d->maxLength) + QLatin1Char(')');
                else
                    fieldSql += QLatin1String(" varchar(") + QString::number(field.d->maxLength) + QLatin1Char(')');
            } else {
                if (databaseType == QDjangoDatabase::MSSqlServer)
                    fieldSql += QLatin1String(" nvarchar(max)");
                else
                    fieldSql += QLatin1String(" text");
            }
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
            if (databaseType == QDjangoDatabase::SQLite)
                // NOTE: django does not add this option for sqlite, but there
                // is a ticket asking for it to do so:
                // https://code.djangoproject.com/ticket/10164
                fieldSql += QLatin1String(" AUTOINCREMENT");
            else if (databaseType == QDjangoDatabase::MySqlServer)
                fieldSql += QLatin1String(" AUTO_INCREMENT");
            else if (databaseType == QDjangoDatabase::PostgreSQL)
                fieldSql = driver->escapeIdentifier(field.column(), QSqlDriver::FieldName) + QLatin1String(" serial PRIMARY KEY");
            else if (databaseType == QDjangoDatabase::MSSqlServer)
                fieldSql += QLatin1String(" IDENTITY(1,1)");
        }

        // foreign key
        if (!field.d->foreignModel.isEmpty())
        {
            const QDjangoMetaModel foreignMeta = QDjango::metaModel(field.d->foreignModel);
            const QDjangoMetaField foreignField = foreignMeta.localField("pk");
            if (databaseType == QDjangoDatabase::MySqlServer) {
                QString constraintName = QString::fromLatin1("FK_%1_%2").arg(
                    field.column(), stringlist_digest(QStringList() << field.column() << d->table));
                QString constraint =
                    QString::fromLatin1("CONSTRAINT %1 FOREIGN KEY (%2) REFERENCES %3 (%4)").arg(
                        driver->escapeIdentifier(constraintName, QSqlDriver::FieldName),
                        driver->escapeIdentifier(field.column(), QSqlDriver::FieldName),
                        driver->escapeIdentifier(foreignMeta.d->table, QSqlDriver::TableName),
                        driver->escapeIdentifier(foreignField.column(), QSqlDriver::FieldName)
                    );

                if (field.d->deleteConstraint != NoAction) {
                    constraint += " ON DELETE";
                    switch (field.d->deleteConstraint) {
                    case Cascade:
                        constraint += " CASCADE";
                        break;
                    case SetNull:
                        constraint += " SET NULL";
                        break;
                    case Restrict:
                        constraint += " RESTRICT";
                        break;
                    default:
                        break;
                    }
                }

                constraintSql << constraint;
            } else {
                fieldSql += QString::fromLatin1(" REFERENCES %1 (%2)").arg(
                    driver->escapeIdentifier(foreignMeta.d->table, QSqlDriver::TableName),
                    driver->escapeIdentifier(foreignField.column(), QSqlDriver::FieldName));

                if (databaseType == QDjangoDatabase::MSSqlServer &&
                    field.d->deleteConstraint == Restrict) {
                    qWarning("MSSQL does not support RESTRICT constraints");
                    break;
                }

                if (field.d->deleteConstraint != NoAction) {
                    fieldSql += " ON DELETE";
                    switch (field.d->deleteConstraint) {
                    case Cascade:
                        fieldSql += " CASCADE";
                        break;
                    case SetNull:
                        fieldSql += " SET NULL";
                        break;
                    case Restrict:
                        fieldSql += " RESTRICT";
                        break;
                    default:
                        break;
                    }
                }
            }

            if (databaseType == QDjangoDatabase::PostgreSQL)
                fieldSql += " DEFERRABLE INITIALLY DEFERRED";
        }
        propSql << fieldSql;
    }

    // add constraints if we need them
    if (!constraintSql.isEmpty())
        propSql << constraintSql.join(QLatin1String(", "));

    // unique contraints
    if (!d->uniqueTogether.isEmpty()) {
        QStringList columns;
        foreach (const QByteArray &name, d->uniqueTogether) {
            columns << driver->escapeIdentifier(localField(name).column(), QSqlDriver::FieldName);
        }
        propSql << QString::fromLatin1("UNIQUE (%2)").arg(columns.join(QLatin1String(", ")));
    }

    // create table
    queries << QString::fromLatin1("CREATE TABLE %1 (%2)").arg(
            quotedTable,
            propSql.join(QLatin1String(", ")));

    // create indices
    foreach (const QDjangoMetaField &field, d->localFields) {
        if (field.d->index) {
            const QString indexName = d->table + QLatin1Char('_')
                + stringlist_digest(QStringList() << field.column());
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

    \return true if the table was dropped or did not exist, false otherwise.
*/
bool QDjangoMetaModel::dropTable() const
{
    QSqlDatabase db = QDjango::database();
    if (!db.tables().contains(d->table))
        return true;

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
    // check the name is valid
    const QByteArray prop(name);
    if (!d->foreignFields.contains(prop)) {
        qWarning("QDjangoMetaModel cannot get foreign model for invalid key '%s'", name);
        return 0;
    }

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
*/
void QDjangoMetaModel::setForeignKey(QObject *model, const char *name, QObject *value) const
{
    // check the name is valid
    const QByteArray prop(name);
    if (!d->foreignFields.contains(prop)) {
        qWarning("QDjangoMetaModel cannot set foreign model for invalid key '%s'", name);
        return;
    }

    QObject *old = model->property(prop + "_ptr").value<QObject*>();
    if (old == value)
        return;

    // store the new pointer and update the foreign key
    model->setProperty(prop + "_ptr", qVariantFromValue(value));
    if (value) {
        const QDjangoMetaModel foreignMeta = QDjango::metaModel(d->foreignFields[prop]);
        model->setProperty(prop + "_id", value->property(foreignMeta.primaryKey()));
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
    QDjangoQuerySetPrivate qs(model->metaObject()->className());
    if (primaryKey.d->autoIncrement) {
        // fetch autoincrement pk
        QVariant insertId;
        if (!qs.sqlInsert(fields, &insertId))
            return false;
        model->setProperty(d->primaryKey, insertId);
    } else {
        if (!qs.sqlInsert(fields))
            return false;
    }
    return true;
}

