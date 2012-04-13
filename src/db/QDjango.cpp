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

#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QMetaProperty>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>
#include <QThread>

#include "QDjango.h"
#include "QDjangoQuerySet_p.h"
#include "QDjangoModel.h"

static const char *connectionPrefix = "_qdjango_";

QMap<QString, QDjangoMetaModel> globalMetaModels = QMap<QString, QDjangoMetaModel>();
static QDjangoDatabase *globalDatabase = 0;
static bool globalDebugEnabled = false;

QDjangoDatabase::QDjangoDatabase(QObject *parent)
    : QObject(parent), connectionId(0)
{
}

void QDjangoDatabase::threadFinished()
{
    QThread *thread = qobject_cast<QThread*>(sender());
    if (!thread)
        return;

    // cleanup database connection for the thread
    QMutexLocker locker(&mutex);
    disconnect(thread, SIGNAL(finished()), this, SLOT(threadFinished()));
    const QString connectionName = copies.value(thread).connectionName();
    copies.remove(thread);
    if (connectionName.startsWith(QLatin1String(connectionPrefix)))
        QSqlDatabase::removeDatabase(connectionName);
}

static void closeDatabase()
{
    delete globalDatabase;
}

QDjangoQuery::QDjangoQuery(QSqlDatabase db)
    : QSqlQuery(db)
{
}

void QDjangoQuery::addBindValue(const QVariant &val, QSql::ParamType paramType)
{
    // this hack is required so that we do not store a mix of local
    // and UTC times
    if (val.type() == QVariant::DateTime)
        QSqlQuery::addBindValue(val.toDateTime().toLocalTime(), paramType);
    else
        QSqlQuery::addBindValue(val, paramType);
}

bool QDjangoQuery::exec()
{
    if (globalDebugEnabled) {
        qDebug() << "SQL query" << lastQuery();
        QMapIterator<QString, QVariant> i(boundValues());
        while (i.hasNext()) {
            i.next();
            qDebug() << "SQL   " << i.key().toAscii().data() << "="
                     << i.value().toString().toAscii().data();
        }
    }
    if (!QSqlQuery::exec()) {
        if (globalDebugEnabled)
            qWarning() << "SQL error" << lastError();
        return false;
    }
    return true;
}

bool QDjangoQuery::exec(const QString &query)
{
    if (globalDebugEnabled)
        qDebug() << "SQL query" << query;
    if (!QSqlQuery::exec(query)) {
        if (globalDebugEnabled)
            qWarning() << "SQL error" << lastError();
        return false;
    }
    return true;
}

/*! \mainpage
 *
 * QDjango is a simple yet powerful Object Relation Mapper (ORM) built
 * on top of the Qt library. Where possible it tries to follow django's
 * ORM API, hence its name.
 *
 * \sa QDjango
 * \sa QDjangoModel
 * \sa QDjangoWhere
 * \sa QDjangoQuerySet
 */


/** Returns the database used by QDjango.
 *
 *  If you call this method from any thread but the application's main thread,
 *  a new connection to the database will be created. The connection will
 *  automatically be torn down once the thread finishes.
 *
 *  \sa setDatabase()
 */
QSqlDatabase QDjango::database()
{
    Q_ASSERT(globalDatabase != 0);
    QThread *thread = QThread::currentThread();

    // if we are in the main thread, return reference connection
    if (thread == globalDatabase->thread())
        return globalDatabase->reference;

    // if we have a connection for this thread, return it
    QMutexLocker locker(&globalDatabase->mutex);
    if (globalDatabase->copies.contains(thread))
        return globalDatabase->copies[thread];

    // create a new connection for this thread
    QObject::connect(thread, SIGNAL(finished()), globalDatabase, SLOT(threadFinished()));
    QSqlDatabase db = QSqlDatabase::cloneDatabase(globalDatabase->reference,
        QLatin1String(connectionPrefix) + QString::number(globalDatabase->connectionId++));
    db.open();
    globalDatabase->copies.insert(thread, db);
    return db;
}

/** Sets the database used by QDjango.
 *
 *  You must call this method from your application's main thread.
 *
 *  \sa database()
 */
void QDjango::setDatabase(QSqlDatabase database)
{
    if (database.driverName() != QLatin1String("QSQLITE") &&
        database.driverName() != QLatin1String("QSQLITE2") &&
        database.driverName() != QLatin1String("QMYSQL") &&
        database.driverName() != QLatin1String("QPSQL"))
    {
        qWarning() << "Unsupported database driver" << database.driverName();
    }
    if (!globalDatabase)
    {
        globalDatabase = new QDjangoDatabase();
        qAddPostRoutine(closeDatabase);
    }
    globalDatabase->reference = database;
}

/** Returns whether debugging information should be printed.
 *
 * \sa setDebugEnabled()
 */
bool QDjango::isDebugEnabled()
{
    return globalDebugEnabled;
}

/** Sets whether debugging information should be printed.
 *
 * \sa isDebugEnabled()
 */
void QDjango::setDebugEnabled(bool enabled)
{
    globalDebugEnabled = enabled;
}

/** Creates the database tables for all registered models.
 */
bool QDjango::createTables()
{
    bool ret = true;
    foreach (const QString &key, globalMetaModels.keys())
        if (!globalMetaModels[key].createTable())
            ret = false;
    return ret;
}

/** Drops the database tables for all registered models.
 */
bool QDjango::dropTables()
{
    bool ret = true;
    foreach (const QString &key, globalMetaModels.keys())
        if (!globalMetaModels[key].dropTable())
            ret = false;
    return ret;
}

/** Returns the QDjangoMetaModel with the given name.
 *
 * \param name
 */
QDjangoMetaModel QDjango::metaModel(const QString &name)
{
    return globalMetaModels.value(name);
}

QDjangoMetaModel QDjango::registerModel(const QObject *model)
{
    const QString name = model->metaObject()->className();
    if (!globalMetaModels.contains(name))
        globalMetaModels.insert(name, QDjangoMetaModel(model));
    return globalMetaModels[name];
}

/** Returns the empty SQL limit clause.
 */
QString QDjango::noLimitSql()
{
    const QString driverName = QDjango::database().driverName();
    if (driverName == QLatin1String("QSQLITE") ||
        driverName == QLatin1String("QSQLITE2"))
        return QLatin1String(" LIMIT -1");
    else if (driverName == QLatin1String("QMYSQL"))
        // 2^64 - 1, as recommended by the MySQL documentation
        return QLatin1String(" LIMIT 18446744073709551615");
    else
        return QString();
}

QDjangoMetaField::QDjangoMetaField()
    : autoIncrement(false),
    index(false),
    maxLength(0),
    unique(false)
{
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

/** Constructs a new QDjangoMetaModel by inspecting the given model instance.
 *
 * \param model
 */
QDjangoMetaModel::QDjangoMetaModel(const QObject *model)
{
    if (!model)
        return;

    const QMetaObject* meta = model->metaObject();
    m_table = QString(meta->className()).toLower().toLatin1();

    // parse table options
    const int optionsIndex = meta->indexOfClassInfo("__meta__");
    if (optionsIndex >= 0) {
        QMap<QString, QString> options = parseOptions(meta->classInfo(optionsIndex).value());
        QMapIterator<QString, QString> option(options);
        while (option.hasNext()) {
            option.next();
            if (option.key() == "db_table")
                m_table = option.value();
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
            m_foreignFields.insert(fkName, fkModel);

            QDjangoMetaField field;
            field.name = fkName + "_id";
            // FIXME : the key is not necessarily an INTEGER field, we should
            // probably perform a lookup on the foreign model, but are we sure
            // it is already registered?
            field.type = QVariant::Int;
            field.foreignModel = fkModel;
            field.index = true;
            m_localFields << field;
            continue;
        }

        // local field
        QDjangoMetaField field;
        field.index = dbIndexOption;
        field.name = meta->property(i).name();
        field.type = meta->property(i).type();
        field.maxLength = maxLengthOption;
        if (primaryKeyOption) {
            field.autoIncrement = autoIncrementOption;
            field.unique = true;
            m_primaryKey = field.name;
        } else if (uniqueOption) {
            field.unique = true;
        }

        m_localFields << field;
    }

    // automatic primary key
    if (m_primaryKey.isEmpty()) {
        QDjangoMetaField field;
        field.name = "id";
        field.type = QVariant::Int;
        field.autoIncrement = true;
        field.unique = true;
        m_localFields.prepend(field);
        m_primaryKey = field.name;
    }
 
}

/** Creates the database table for this QDjangoMetaModel.
 */
bool QDjangoMetaModel::createTable() const
{
    QSqlDatabase db = QDjango::database();
    QSqlDriver *driver = db.driver();
    const QString driverName = db.driverName();

    QStringList propSql;
    const QString quotedTable = db.driver()->escapeIdentifier(m_table, QSqlDriver::TableName);
    foreach (const QDjangoMetaField &field, m_localFields)
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

        // primary key
        if (field.name == m_primaryKey)
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
                driver->escapeIdentifier(foreignMeta.m_table, QSqlDriver::TableName),
                driver->escapeIdentifier(foreignMeta.m_primaryKey, QSqlDriver::FieldName));
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
    foreach (const QDjangoMetaField &field, m_localFields) {
        if (field.index && !field.unique) {
            const QString indexName = m_table + "_" + QString::fromLatin1(field.name);
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

/** Drops the database table for this QDjangoMetaModel.
 */
bool QDjangoMetaModel::dropTable() const
{
    QSqlDatabase db = QDjango::database();

    QDjangoQuery query(db);
    return query.exec(QString("DROP TABLE %1").arg(
        db.driver()->escapeIdentifier(m_table, QSqlDriver::TableName)));
}

/** Retrieves the QDjangoModel pointed to by the given foreign-key.
 *
 * \param model
 * \param name
 */
QObject *QDjangoMetaModel::foreignKey(const QObject *model, const char *name) const
{
    const QByteArray prop(name);
    QObject *foreign = model->property(prop + "_ptr").value<QObject*>();
    if (!foreign)
        return 0;

    // if the foreign object was not loaded yet, do it now
    const QString foreignClass = m_foreignFields[prop];
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

/** Sets the QDjangoModel pointed to by the given foreign-key.
 *
 * \param model
 * \param name
 * \param value
 *
 * \note The \c model will take ownership of the given \c value.
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
        const QDjangoMetaModel foreignMeta = QDjango::metaModel(m_foreignFields[prop]);
        model->setProperty(prop + "_id", value->property(foreignMeta.primaryKey()));
        value->setParent(model);
    } else {
        model->setProperty(prop + "_id", QVariant());
    }
}


/** Returns true if the current QDjangoMetaModel is valid, false otherwise.
 */
bool QDjangoMetaModel::isValid() const
{
    return !m_table.isEmpty() && !m_primaryKey.isEmpty();
}

/** Loads the given properties into a model instance.
 *
 * \param model
 * \param properties
 * \param pos
 */
void QDjangoMetaModel::load(QObject *model, const QVariantList &properties, int &pos) const
{
    // process local fields
    foreach (const QDjangoMetaField &field, m_localFields)
        model->setProperty(field.name, properties.at(pos++));

    // process foreign fields
    if (pos >= properties.size())
        return;
    foreach (const QByteArray &fkName, m_foreignFields.keys())
    {
        QObject *object = model->property(fkName + "_ptr").value<QObject*>();
        if (object)
        {
            const QDjangoMetaModel foreignMeta = QDjango::metaModel(m_foreignFields[fkName]);
            foreignMeta.load(object, properties, pos);
        }
    }
}

/** Returns the foreign field mapping.
 */
QMap<QByteArray, QString> QDjangoMetaModel::foreignFields() const
{
    return m_foreignFields;
}

/** Returns the list of local fields.
 */
QList<QDjangoMetaField> QDjangoMetaModel::localFields() const
{
    return m_localFields;
}

/** Returns the name of the primary key for the current QDjangoMetaModel.
 */
QByteArray QDjangoMetaModel::primaryKey() const
{
    return m_primaryKey;
}

/** Returns the name of the database table.
 */
QString QDjangoMetaModel::table() const
{
    return m_table;
}

/** Removes the given QObject from the database.
 *
 * \param model
 */
bool QDjangoMetaModel::remove(QObject *model) const
{
    QSqlDatabase db = QDjango::database();

    QDjangoQuery query(db);
    query.prepare(QString("DELETE FROM %1 WHERE %2 = ?").arg(
                  db.driver()->escapeIdentifier(m_table, QSqlDriver::TableName),
                  db.driver()->escapeIdentifier(m_primaryKey, QSqlDriver::FieldName)));
    query.addBindValue(model->property(m_primaryKey));
    return query.exec();
}

/** Saves the given QObject to the database.
 *
 * \param model
 *
 * \return true if saving succeeded, false otherwise
 */
bool QDjangoMetaModel::save(QObject *model) const
{
    QSqlDatabase db = QDjango::database();
    QSqlDriver *driver = db.driver();

    QStringList fieldNames;
    QDjangoMetaField primaryKey;
    foreach (const QDjangoMetaField &field, m_localFields)
    {
        if (field.name == m_primaryKey)
            primaryKey = field;
        fieldNames << field.name;
    }

    const QString quotedTable = db.driver()->escapeIdentifier(m_table, QSqlDriver::TableName);
    const QVariant pk = model->property(primaryKey.name);
    if (!pk.isNull() && !(primaryKey.type == QVariant::Int && !pk.toInt()))
    {
        QDjangoQuery query(db);
        query.prepare(QString("SELECT 1 AS a FROM %1 WHERE %2 = ?").arg(
                      quotedTable,
                      driver->escapeIdentifier(primaryKey.name, QSqlDriver::FieldName)));
        query.addBindValue(pk);
        if (query.exec() && query.next())
        {
            // remove primary key
            fieldNames.removeAll(primaryKey.name);

            // perform update
            QStringList fieldAssign;
            foreach (const QString &name, fieldNames)
                fieldAssign << driver->escapeIdentifier(name, QSqlDriver::FieldName) + " = ?";

            QDjangoQuery query(db);
            query.prepare(QString("UPDATE %1 SET %2 WHERE %3 = ?")
                  .arg(quotedTable, fieldAssign.join(", "), primaryKey.name));
            foreach (const QString &name, fieldNames)
                query.addBindValue(model->property(name.toLatin1()));
            query.addBindValue(pk);
            return query.exec();
        }
    }

    // remove auto-increment column
    if (primaryKey.autoIncrement)
        fieldNames.removeAll(primaryKey.name);

    // perform insert
    QStringList fieldColumns;
    QStringList fieldHolders;
    foreach (const QString &name, fieldNames)
    {
        fieldColumns << driver->escapeIdentifier(name, QSqlDriver::FieldName);
        fieldHolders << "?";
    }

    QDjangoQuery query(db);
    query.prepare(QString("INSERT INTO %1 (%2) VALUES(%3)").arg(
                  quotedTable,
                  fieldColumns.join(", "), fieldHolders.join(", ")));
    foreach (const QString &name, fieldNames)
        query.addBindValue(model->property(name.toLatin1()));

    bool ret = query.exec();
    if (primaryKey.autoIncrement) {
        QVariant insertId;
        if (db.driverName() == "QPSQL") {
            QDjangoQuery query(db);
            const QString seqName = driver->escapeIdentifier(m_table + "_" + primaryKey.name + "_seq", QSqlDriver::FieldName);
            if (!query.exec("SELECT CURRVAL('" + seqName + "')") || !query.next())
                return false;
            insertId = query.value(0);
        } else {
            insertId = query.lastInsertId();
        }
        model->setProperty(primaryKey.name, insertId);
    }
    return ret;
}


