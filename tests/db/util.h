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

#include <QtTest>

#include "QDjango.h"
#include "QDjangoModel.h"

bool initialiseDatabase();
QString normalizeSql(const QSqlDatabase &db, const QString &sql);

#define CHECKWHERE(_where, s, v) { \
    QSqlDatabase _sql_db(QDjango::database()); \
    QDjangoQuery _sql_query(_sql_db); \
    QString _sql_stmt = _where.sql(_sql_db); \
    if (!_sql_stmt.isEmpty()) _sql_query.prepare(_sql_stmt); \
    _where.bindValues(_sql_query); \
    const QVariantList _sql_values = v; \
    QCOMPARE(normalizeSql(_sql_db, _sql_query.lastQuery()), s); \
    QCOMPARE(_sql_query.boundValues().size(), _sql_values.size()); \
    for(int _i = 0; _i < _sql_values.size(); ++_i) QCOMPARE(_sql_query.boundValue(_i), _sql_values[_i]); \
    }

