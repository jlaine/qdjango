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

#ifndef QDJANGO_QUERYSET_P_H
#define QDJANGO_QUERYSET_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QDjango API.
//

#include <QStringList>

#include "QDjango_p.h"
#include "QDjangoWhere.h"

class QDjangoMetaModel;

/** \internal
 */
class QDJANGO_EXPORT QDjangoCompiler
{
public:
    QDjangoCompiler(const char *modelName, const QSqlDatabase &db);
    QString fromSql();
    QStringList fieldNames(bool recurse, QDjangoMetaModel *metaModel = 0, const QString &modelPath = QString());
    QString orderLimitSql(const QStringList orderBy, int lowMark, int highMark);
    void resolve(QDjangoWhere &where);

private:
    QString databaseColumn(const QString &name);
    QString referenceModel(const QString &modelPath, QDjangoMetaModel *metaModel);

    QSqlDriver *driver;
    QDjangoMetaModel baseModel;
    QMap<QString, QPair<QString, QDjangoMetaModel> > modelRefs;
    QMap<QString, QString> fieldColumnCache;
};

/** \internal
 */
class QDJANGO_EXPORT QDjangoQuerySetPrivate
{
public:
    QDjangoQuerySetPrivate(const char *modelName);

    void addFilter(const QDjangoWhere &where);
    QDjangoWhere resolvedWhere(const QSqlDatabase &db) const;
    bool sqlDelete();
    bool sqlFetch();
    bool sqlInsert(const QVariantMap &fields, QVariant *insertId = 0);
    bool sqlLoad(QObject *model, int index);
    int sqlUpdate(const QVariantMap &fields);
    QList<QVariantMap> sqlValues(const QStringList &fields);
    QList<QVariantList> sqlValuesList(const QStringList &fields);

    // SQL queries
    QDjangoQuery countQuery() const;
    QDjangoQuery deleteQuery() const;
    QDjangoQuery insertQuery(const QVariantMap &fields) const;
    QDjangoQuery updateQuery(const QVariantMap &fields) const;

    // reference counter
    QAtomicInt counter;

    bool hasResults;
    int lowMark;
    int highMark;
    QDjangoWhere whereClause;
    QStringList orderBy;
    QList<QVariantList> properties;
    bool selectRelated;

private:
    Q_DISABLE_COPY(QDjangoQuerySetPrivate)

    QByteArray m_modelName;

    friend class QDjangoMetaModel;
};

#endif
