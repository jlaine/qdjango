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
    /** A comparison operation on a database column value. */
    enum Operation
    {
        /** No comparison, always returns true. */
        None,
        /** Returns true if the column value is equal to the given value. */
        Equals,
        /** Returns true if the column value is not equal to the given value. */
        NotEquals,
        /** Returns true if the column value is greater than the given value. */
        GreaterThan,
        /** Returns true if the column value is less than the given value. */
        LessThan,
        /** Returns true if the column value is greater or equal to the given value. */
        GreaterOrEquals,
        /** Returns true if the column value is less or equal to the given value. */
        LessOrEquals,
        /** Returns true if the column value starts with the given value (strings only). */
        StartsWith,
        /** Returns true if the column value ends with the given value (strings only). */
        EndsWith,
        /** Returns true if the column value contains the given value (strings only). */
        Contains,
        /** Returns true if the column value is one of the given values. */
        IsIn,
        /** Returns true if the column value is null. */
        IsNull
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

private:
    QSharedDataPointer<QDjangoWherePrivate> d;
    friend class QDjangoCompiler;
};

#endif
