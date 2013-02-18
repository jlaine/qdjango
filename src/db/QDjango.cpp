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

#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>
#include <QThread>

#include "QDjango.h"

static const char *connectionPrefix = "_qdjango_";

QMap<QByteArray, QDjangoMetaModel> globalMetaModels = QMap<QByteArray, QDjangoMetaModel>();
static QDjangoDatabase *globalDatabase = 0;
static bool globalDebugEnabled = false;

/// \cond

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

static void initDatabase(QSqlDatabase db)
{
    if (db.driverName() == QLatin1String("QSQLITE")) {
        // enable foreign key constraint handling
        QDjangoQuery query(db);
        query.prepare("PRAGMA foreign_keys=on");
        query.exec();
    }
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
            qDebug() << "SQL   " << i.key().toLatin1().data() << "="
                     << i.value().toString().toLatin1().data();
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

/// \endcond

/*!
    \mainpage

    QDjango is a simple yet powerful Object Relation Mapper (ORM) built
    on top of the Qt library. Where possible it tries to follow django's
    ORM API, hence its name.

    \sa QDjango
    \sa QDjangoModel
    \sa QDjangoWhere
    \sa QDjangoQuerySet
*/

/*!
    Returns the database used by QDjango.

    If you call this method from any thread but the application's main thread,
    a new connection to the database will be created. The connection will
    automatically be torn down once the thread finishes.

    \sa setDatabase()
*/
QSqlDatabase QDjango::database()
{
    if (!globalDatabase)
        return QSqlDatabase();

    // if we are in the main thread, return reference connection
    QThread *thread = QThread::currentThread();
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
    initDatabase(db);
    globalDatabase->copies.insert(thread, db);
    return db;
}

/*!
    Sets the database used by QDjango.

    You must call this method from your application's main thread.

    \sa database()
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
    initDatabase(database);
    globalDatabase->reference = database;
}

/*!
    Returns whether debugging information should be printed.

    \sa setDebugEnabled()
*/
bool QDjango::isDebugEnabled()
{
    return globalDebugEnabled;
}

/*!
    Sets whether debugging information should be printed.

    \sa isDebugEnabled()
*/
void QDjango::setDebugEnabled(bool enabled)
{
    globalDebugEnabled = enabled;
}

/*!
    Creates the database tables for all registered models.
*/
bool QDjango::createTables()
{
    bool ret = true;
    foreach (const QByteArray &key, globalMetaModels.keys())
        if (!globalMetaModels[key].createTable())
            ret = false;
    return ret;
}

/*!
    Drops the database tables for all registered models.
*/
bool QDjango::dropTables()
{
    bool ret = true;
    foreach (const QByteArray &key, globalMetaModels.keys())
        if (!globalMetaModels[key].dropTable())
            ret = false;
    return ret;
}

/*!
    Returns the QDjangoMetaModel with the given \a name.
 */
QDjangoMetaModel QDjango::metaModel(const char *name)
{
    return globalMetaModels.value(name);
}

QDjangoMetaModel QDjango::registerModel(const QMetaObject *meta)
{
    const QByteArray name = meta->className();
    if (!globalMetaModels.contains(name))
        globalMetaModels.insert(name, QDjangoMetaModel(meta));
    return globalMetaModels[name];
}

/*!
    Returns the empty SQL limit clause.
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

