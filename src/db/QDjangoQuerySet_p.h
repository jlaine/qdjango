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

#ifndef QDJANGO_QUERYSET_P_H
#define QDJANGO_QUERYSET_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QDjango API.
//

#include <QStringList>

#include "QDjangoWhere.h"

class QDjangoMetaModel;

/** \internal
 */
class QDjangoCompiler
{
public:
    QDjangoCompiler(const QString &modelName, const QSqlDatabase &db);
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
};

/** \internal
 */
class QDjangoQuerySetPrivate
{
public:
    QDjangoQuerySetPrivate(const QString &modelName);

    void addFilter(const QDjangoWhere &where);
    QDjangoWhere resolvedWhere(const QSqlDatabase &db) const;
    int sqlCount() const;
    bool sqlDelete();
    bool sqlFetch();
    bool sqlLoad(QObject *model, int index);
    int sqlUpdate(const QVariantMap &fields);
    QList<QVariantMap> sqlValues(const QStringList &fields);
    QList<QVariantList> sqlValuesList(const QStringList &fields);

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

    QString m_modelName;

    friend class QDjangoMetaModel;
};

#endif
