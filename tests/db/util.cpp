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

#include <cstdlib>

#include <QtTest>

#include "QDjango.h"

#include "util.h"

void initialiseDatabase()
{
    char *p;

    // enable SQL debugging
    QDjango::setDebugEnabled(true);

    // open database
    QString databaseDriver = "QSQLITE";
    p = getenv("QDJANGO_DB_DRIVER");
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

    QVERIFY(db.open());
    QDjango::setDatabase(db);
}

QString normalizeSql(const QSqlDatabase &db, const QString &sql)
{
    const QString driverName = db.driverName();
    QString modSql(sql);
    if (driverName == "QMYSQL")
        modSql.replace("`", "\"");
    else if (driverName == "QSQLITE" || driverName == "QSQLITE2")
        modSql.replace("LIKE ? ESCAPE '\\'", "LIKE ?");
    return modSql;
}
