/*
 * Copyright (C) 2010-2015 Jeremy Lainé
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

#include <cstdlib>

#include "QDjango.h"

#include "util.h"

bool initialiseDatabase()
{
    char *p;

    // enable SQL debugging
    if ((p = getenv("QDJANGO_DB_DEBUG")) != 0)
        QDjango::setDebugEnabled(true);

    // open database
    QString databaseDriver = "QSQLITE";
    if ((p = getenv("QDJANGO_DB_DRIVER")) != 0)
        databaseDriver = QString::fromLocal8Bit(p);
    QSqlDatabase db = QSqlDatabase::addDatabase(databaseDriver);

    if ((p = getenv("QDJANGO_DB_NAME")) != 0)
        db.setDatabaseName(QString::fromLocal8Bit(p));
    else if (databaseDriver == "QSQLITE")
        db.setDatabaseName(":memory:");

    if ((p = getenv("QDJANGO_DB_USER")) != 0)
        db.setUserName(QString::fromLocal8Bit(p));

    if ((p = getenv("QDJANGO_DB_PASSWORD")) != 0)
        db.setPassword(QString::fromLocal8Bit(p));

    if ((p = getenv("QDJANGO_DB_HOST")) != 0)
        db.setHostName(QString::fromLocal8Bit(p));

    if (db.open()) {
        QDjango::setDatabase(db);
        return true;
    } else {
        return false;
    }
}

QString normalizeSql(const QSqlDatabase &db, const QString &sql)
{
    QDjangoDatabase::DatabaseType databaseType = QDjangoDatabase::databaseType(db);
    QString modSql(sql);
    if (databaseType == QDjangoDatabase::MySqlServer)
        modSql.replace("`", "\"");
    else if (databaseType == QDjangoDatabase::SQLite)
        modSql.replace("? ESCAPE '\\'", "?");
    return modSql;
}
