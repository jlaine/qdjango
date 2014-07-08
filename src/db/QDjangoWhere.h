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

#ifndef QDJANGO_WHERE_H
#define QDJANGO_WHERE_H

#include <QSharedDataPointer>
#include <QVariant>

#include "QDjango_p.h"

class QDjangoMetaModel;
class QDjangoQuery;
class QDjangoWherePrivate;

/** \brief The QDjangoWhere class expresses an SQL constraint.
 *
 * The QDjangoWhere class is used to build SQL WHERE statements. In its
 * simplest form a QDjangoWhere expresses a constraint on a column value.
 *
 * QDjangoWhere instances can be negated using the "!" unary operator
 * or combined using the "&&" and "||" boolean operators.
 *
 * \ingroup Database
 */
class QDJANGO_EXPORT QDjangoWhere
{
public:
    enum Operation
    {
        None,
        Equals,
        NotEquals,
        GreaterThan,
        LessThan,
        GreaterOrEquals,
        LessOrEquals,
        StartsWith,
        EndsWith,
        Contains,
        IsIn,
        IsNull,
        IEquals,
        INotEquals,
        IStartsWith,
        IEndsWith,
        IContains
    };

    QDjangoWhere();
    QDjangoWhere(const QDjangoWhere &other);
    QDjangoWhere(const QString &key, QDjangoWhere::Operation operation, QVariant value);
    ~QDjangoWhere();

    QDjangoWhere& operator=(const QDjangoWhere &other);
    QDjangoWhere operator!() const;
    QDjangoWhere operator&&(const QDjangoWhere &other) const;
    QDjangoWhere operator||(const QDjangoWhere &other) const;

    void bindValues(QDjangoQuery &query) const;
    bool isAll() const;
    bool isNone() const;
    QString sql(const QSqlDatabase &db) const;
    QString toString() const;

private:
    QSharedDataPointer<QDjangoWherePrivate> d;
    friend class QDjangoCompiler;
};

#endif
