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

#include <QStringList>
#include <QDebug>

#include "QDjango.h"
#include "QDjangoWhere.h"
#include "QDjangoWhere_p.h"

static QString escapeLike(const QString &data)
{
    QString escaped = data;
    escaped.replace(QLatin1String("%"), QLatin1String("\\%"));
    escaped.replace(QLatin1String("_"), QLatin1String("\\_"));
    return escaped;
}

/// \cond

QDjangoWherePrivate::QDjangoWherePrivate()
    : operation(QDjangoWhere::None)
    , combine(NoCombine)
    , negate(false)
{
}

/// \endcond

/*!
    \enum QDjangoWhere::Operation
    A comparison operation on a database column value.

    \var QDjangoWhere::Operation QDjangoWhere::None
    No comparison, always returns true.

    \var QDjangoWhere::Operation QDjangoWhere::Equals
    Returns true if the column value is equal to the given value.

    \var QDjangoWhere::Operation QDjangoWhere::IEquals
    Returns true if the column value is equal to the given value (case-insensitive)

    \var QDjangoWhere::Operation QDjangoWhere::NotEquals
    Returns true if the column value is not equal to the given value.

    \var QDjangoWhere::Operation QDjangoWhere::INotEquals
    Returns true if the column value is not equal to the given value (case-insensitive).

    \var QDjangoWhere::Operation QDjangoWhere::GreaterThan,
    Returns true if the column value is greater than the given value.

    \var QDjangoWhere::Operation QDjangoWhere::LessThan,
    Returns true if the column value is less than the given value.

    \var QDjangoWhere::Operation QDjangoWhere::GreaterOrEquals,
    Returns true if the column value is greater or equal to the given value.

    \var QDjangoWhere::Operation QDjangoWhere::LessOrEquals,
    Returns true if the column value is less or equal to the given value.

    \var QDjangoWhere::Operation QDjangoWhere::StartsWith,
    Returns true if the column value starts with the given value (strings only).

    \var QDjangoWhere::Operation QDjangoWhere::IStartsWith,
    Returns true if the column value starts with the given value (strings only, case-insensitive).

    \var QDjangoWhere::Operation QDjangoWhere::EndsWith,
    Returns true if the column value ends with the given value (strings only).

    \var QDjangoWhere::Operation QDjangoWhere::IEndsWith,
    Returns true if the column value ends with the given value (strings only, case-insensitive).

    \var QDjangoWhere::Operation QDjangoWhere::Contains,
    Returns true if the column value contains the given value (strings only).

    \var QDjangoWhere::Operation QDjangoWhere::IContains,
    Returns true if the column value contains the given value (strings only, case-insensitive).

    \var QDjangoWhere::Operation QDjangoWhere::IsIn,
    Returns true if the column value is one of the given values.

    \var QDjangoWhere::Operation QDjangoWhere::IsNull
    Returns true if the column value is null.
*/

/** Constructs an empty QDjangoWhere, which expresses no constraint.
 */
QDjangoWhere::QDjangoWhere()
{
    d = new QDjangoWherePrivate;
}

/** Constructs a copy of \a other.
 */
QDjangoWhere::QDjangoWhere(const QDjangoWhere &other)
    : d(other.d)
{
}

/** Constructs a QDjangoWhere expressing a constraint on a database column.
 *
 * \param key
 * \param operation
 * \param value
 */
QDjangoWhere::QDjangoWhere(const QString &key, QDjangoWhere::Operation operation, QVariant value)
{
    d = new QDjangoWherePrivate;
    d->key = key;
    d->operation = operation;
    d->data = value;
}

/** Destroys a QDjangoWhere.
 */
QDjangoWhere::~QDjangoWhere()
{
}

/** Assigns \a other to this QDjangoWhere.
 */
QDjangoWhere& QDjangoWhere::operator=(const QDjangoWhere& other)
{
    d = other.d;
    return *this;
}

/** Negates the current QDjangoWhere.
 */
QDjangoWhere QDjangoWhere::operator!() const
{
    QDjangoWhere result;
    result.d = d;
    if (d->children.isEmpty()) {
        switch (d->operation)
        {
        case None:
        case IsIn:
        case StartsWith:
        case IStartsWith:
        case EndsWith:
        case IEndsWith:
        case Contains:
        case IContains:
            result.d->negate = !d->negate;
            break;
        case IsNull:
            // simplify !(is null) to is not null
            result.d->data = !d->data.toBool();
            break;
        case Equals:
            // simplify !(a = b) to a != b
            result.d->operation = NotEquals;
            break;
        case IEquals:
            // simplify !(a = b) to a != b
            result.d->operation = INotEquals;
            break;
        case NotEquals:
            // simplify !(a != b) to a = b
            result.d->operation = Equals;
            break;
        case INotEquals:
            // simplify !(a != b) to a = b
            result.d->operation = IEquals;
            break;
        case GreaterThan:
            // simplify !(a > b) to a <= b
            result.d->operation = LessOrEquals;
            break;
        case LessThan:
            // simplify !(a < b) to a >= b
            result.d->operation = GreaterOrEquals;
            break;
        case GreaterOrEquals:
            // simplify !(a >= b) to a < b
            result.d->operation = LessThan;
            break;
        case LessOrEquals:
            // simplify !(a <= b) to a > b
            result.d->operation = GreaterThan;
            break;
        }
    } else {
        result.d->negate = !d->negate;
    }

    return result;
}

/** Combines the current QDjangoWhere with the \a other QDjangoWhere using
 *  a logical AND.
 *
 * \param other
 */
QDjangoWhere QDjangoWhere::operator&&(const QDjangoWhere &other) const
{
    if (isAll() || other.isNone())
        return other;
    else if (isNone() || other.isAll())
        return *this;

    if (d->combine == QDjangoWherePrivate::AndCombine) {
        QDjangoWhere result = *this;
        result.d->children << other;
        return result;
    }

    QDjangoWhere result;
    result.d->combine = QDjangoWherePrivate::AndCombine;
    result.d->children << *this << other;
    return result;
}

/** Combines the current QDjangoWhere with the \a other QDjangoWhere using
 *  a logical OR.
 *
 * \param other
 */
QDjangoWhere QDjangoWhere::operator||(const QDjangoWhere &other) const
{
    if (isAll() || other.isNone())
        return *this;
    else if (isNone() || other.isAll())
        return other;

    if (d->combine == QDjangoWherePrivate::OrCombine) {
        QDjangoWhere result = *this;
        result.d->children << other;
        return result;
    }

    QDjangoWhere result;
    result.d->combine = QDjangoWherePrivate::OrCombine;
    result.d->children << *this << other;
    return result;
}

/** Bind the values associated with this QDjangoWhere to the given \a query.
 *
 * \param query
 */
void QDjangoWhere::bindValues(QDjangoQuery &query) const
{
    if (d->operation == QDjangoWhere::IsIn) {
        const QList<QVariant> values = d->data.toList();
        for (int i = 0; i < values.size(); i++)
            query.addBindValue(values[i]);
    } else if (d->operation == QDjangoWhere::IsNull) {
        // no data to bind
    } else if (d->operation == QDjangoWhere::StartsWith || d->operation == QDjangoWhere::IStartsWith) {
        query.addBindValue(escapeLike(d->data.toString()) + QLatin1String("%"));
    } else if (d->operation == QDjangoWhere::EndsWith || d->operation == QDjangoWhere::IEndsWith) {
        query.addBindValue(QLatin1String("%") + escapeLike(d->data.toString()));
    } else if (d->operation == QDjangoWhere::Contains || d->operation == QDjangoWhere::IContains) {
        query.addBindValue(QLatin1String("%") + escapeLike(d->data.toString()) + QLatin1String("%"));
    } else if (d->operation != QDjangoWhere::None) {
        query.addBindValue(d->data);
    } else {
        foreach (const QDjangoWhere &child, d->children)
            child.bindValues(query);
    }
}

/** Returns true if the current QDjangoWhere does not express any constraint.
 */
bool QDjangoWhere::isAll() const
{
    return d->combine == QDjangoWherePrivate::NoCombine && d->operation == None && d->negate == false;
}

/** Returns true if the current QDjangoWhere expressed an impossible constraint.
 */
bool QDjangoWhere::isNone() const
{
    return d->combine == QDjangoWherePrivate::NoCombine && d->operation == None && d->negate == true;
}

/** Returns the SQL code corresponding for the current QDjangoWhere.
 */
/* Note - SQLite is always case-insensitive because it can't figure out case when using non-Ascii charcters:
        https://docs.djangoproject.com/en/dev/ref/databases/#sqlite-string-matching
   Note - MySQL is only case-sensitive when the collation is set as such:
        https://code.djangoproject.com/ticket/9682

 */
QString QDjangoWhere::sql(const QSqlDatabase &db) const
{
    QDjangoDatabase::DatabaseType databaseType = QDjangoDatabase::databaseType(db);

    switch (d->operation) {
        case Equals:
            return d->key + QLatin1String(" = ?");
        case NotEquals:
            return d->key + QLatin1String(" != ?");
        case GreaterThan:
            return d->key + QLatin1String(" > ?");
        case LessThan:
            return d->key + QLatin1String(" < ?");
        case GreaterOrEquals:
            return d->key + QLatin1String(" >= ?");
        case LessOrEquals:
            return d->key + QLatin1String(" <= ?");
        case IsIn:
        {
            QStringList bits;
            for (int i = 0; i < d->data.toList().size(); i++)
                bits << QLatin1String("?");
            if (d->negate)
                return d->key + QString::fromLatin1(" NOT IN (%1)").arg(bits.join(QLatin1String(", ")));
            else
                return d->key + QString::fromLatin1(" IN (%1)").arg(bits.join(QLatin1String(", ")));
        }
        case IsNull:
            return d->key + QLatin1String(d->data.toBool() ? " IS NULL" : " IS NOT NULL");
        case StartsWith:
        case EndsWith:
        case Contains:
        {
            QString op;
            if (databaseType == QDjangoDatabase::MySqlServer)
                op = QLatin1String(d->negate ? "NOT LIKE BINARY" : "LIKE BINARY");
            else
                op = QLatin1String(d->negate ? "NOT LIKE" : "LIKE");
            if (databaseType == QDjangoDatabase::SQLite)
                return d->key + QLatin1String(" ") + op + QLatin1String(" ? ESCAPE '\\'");
            else
                return d->key + QLatin1String(" ") + op + QLatin1String(" ?");
        }
        case IStartsWith:
        case IEndsWith:
        case IContains:
        case IEquals:
        {
            const QString op = QLatin1String(d->negate ? "NOT LIKE" : "LIKE");
            if (databaseType == QDjangoDatabase::SQLite)
                return d->key + QLatin1String(" ") + op + QLatin1String(" ? ESCAPE '\\'");
            else if (databaseType == QDjangoDatabase::PostgreSQL)
                return QLatin1String("UPPER(") + d->key + QLatin1String("::text) ") + op + QLatin1String(" UPPER(?)");
            else
                return d->key + QLatin1String(" ") + op + QLatin1String(" ?");
        }
        case INotEquals:
        {
            const QString op = QLatin1String(d->negate ? "LIKE" : "NOT LIKE");
            if (databaseType == QDjangoDatabase::SQLite)
                return d->key + QLatin1String(" ") + op + QLatin1String(" ? ESCAPE '\\'");
            else if (databaseType == QDjangoDatabase::PostgreSQL)
                return QLatin1String("UPPER(") + d->key + QLatin1String("::text) ") + op + QLatin1String(" UPPER(?)");
            else
                return d->key + QLatin1String(" ") + op + QLatin1String(" ?");
        }
        case None:
            if (d->combine == QDjangoWherePrivate::NoCombine) {
                return d->negate ? QLatin1String("1 != 0") : QString();
            } else {
                QStringList bits;
                foreach (const QDjangoWhere &child, d->children) {
                    QString atom = child.sql(db);
                    if (child.d->children.isEmpty())
                        bits << atom;
                    else
                        bits << QString::fromLatin1("(%1)").arg(atom);
                }

                QString combined;
                if (d->combine == QDjangoWherePrivate::AndCombine)
                    combined = bits.join(QLatin1String(" AND "));
                else if (d->combine == QDjangoWherePrivate::OrCombine)
                    combined = bits.join(QLatin1String(" OR "));
                if (d->negate)
                    combined = QString::fromLatin1("NOT (%1)").arg(combined);
                return combined;
            }
    }

    return QString();
}

QString QDjangoWhere::toString() const
{
    if (d->combine == QDjangoWherePrivate::NoCombine) {
        return QLatin1String("QDjangoWhere(")
                  + "key=\"" + d->key + "\""
                  + ", operation=\"" + QDjangoWherePrivate::operationToString(d->operation) + "\""
                  + ", value=\"" + d->data.toString() + "\""
                  + ", negate=" + (d->negate ? "true":"false")
                  + ")";
    } else {
        QStringList bits;
        foreach (const QDjangoWhere &childWhere, d->children) {
            bits.append(childWhere.toString());
        }
        if (d->combine == QDjangoWherePrivate::OrCombine) {
            return bits.join(" || ");
        } else {
            return bits.join(" && ");
        }
    }
}
QString QDjangoWherePrivate::operationToString(QDjangoWhere::Operation operation)
{
    switch (operation) {
    case QDjangoWhere::Equals: return QLatin1String("Equals");
    case QDjangoWhere::IEquals: return QLatin1String("IEquals");
    case QDjangoWhere::NotEquals: return QLatin1String("NotEquals");
    case QDjangoWhere::INotEquals: return QLatin1String("INotEquals");
    case QDjangoWhere::GreaterThan: return QLatin1String("GreaterThan");
    case QDjangoWhere::LessThan: return QLatin1String("LessThan");
    case QDjangoWhere::GreaterOrEquals: return QLatin1String("GreaterOrEquals");
    case QDjangoWhere::LessOrEquals: return QLatin1String("LessOrEquals");
    case QDjangoWhere::StartsWith: return QLatin1String("StartsWith");
    case QDjangoWhere::IStartsWith: return QLatin1String("IStartsWith");
    case QDjangoWhere::EndsWith: return QLatin1String("EndsWith");
    case QDjangoWhere::IEndsWith: return QLatin1String("IEndsWith");
    case QDjangoWhere::Contains: return QLatin1String("Contains");
    case QDjangoWhere::IContains: return QLatin1String("IContains");
    case QDjangoWhere::IsIn: return QLatin1String("IsIn");
    case QDjangoWhere::IsNull: return QLatin1String("IsNull");
    case QDjangoWhere::None:
    default:
        return QLatin1String("");
    }

    return QString();
}
