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

#include <QStringList>

#include "QDjango.h"
#include "QDjangoWhere.h"
#include "QDjangoWhere_p.h"

/// \cond

QDjangoWherePrivate::QDjangoWherePrivate()
    : operation(QDjangoWhere::None)
    , combine(NoCombine)
    , negate(false)
{
}

/// \endcond

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
    if (d->children.isEmpty())
    {
        switch (d->operation)
        {
        case None:
        case IsIn:
        case StartsWith:
        case EndsWith:
        case Contains:
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
        case NotEquals:
            // simplify !(a != b) to a = b
            result.d->operation = Equals;
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
    if (d->operation == QDjangoWhere::IsIn)
    {
        const QList<QVariant> values = d->data.toList();
        for (int i = 0; i < values.size(); i++)
            query.addBindValue(values[i]);
    }
    else if (d->operation == QDjangoWhere::IsNull)
    {
        // no data to bind
    }
    else if (d->operation == QDjangoWhere::StartsWith)
    {
        QString escaped = d->data.toString();
        escaped.replace("%", "\\%");
        escaped.replace("_", "\\_");
        query.addBindValue(escaped + "%");
    }
    else if (d->operation == QDjangoWhere::EndsWith)
    {
        QString escaped = d->data.toString();
        escaped.replace("%", "\\%");
        escaped.replace("_", "\\_");
        query.addBindValue("%" + escaped);
    }
    else if (d->operation == QDjangoWhere::Contains)
    {
        QString escaped = d->data.toString();
        escaped.replace("%", "\\%");
        escaped.replace("_", "\\_");
        query.addBindValue("%" + escaped + "%");
    }
    else if (d->operation != QDjangoWhere::None)
        query.addBindValue(d->data);
    else
        foreach (const QDjangoWhere &child, d->children)
            child.bindValues(query);
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
QString QDjangoWhere::sql(const QSqlDatabase &db) const
{
    switch (d->operation)
    {
        case Equals:
            return d->key + " = ?";
        case NotEquals:
            return d->key + " != ?";
        case GreaterThan:
            return d->key + " > ?";
        case LessThan:
            return d->key + " < ?";
        case GreaterOrEquals:
            return d->key + " >= ?";
        case LessOrEquals:
            return d->key + " <= ?";
        case IsIn:
        {
            QStringList bits;
            for (int i = 0; i < d->data.toList().size(); i++)
                bits << "?";
            return d->key + (d->negate ? " NOT IN " : " IN ") + "(" + bits.join(", ") + ")";
        }
        case IsNull:
            return d->key + (d->data.toBool() ? " IS NULL" : " IS NOT NULL");
        case StartsWith:
        case EndsWith:
        case Contains:
        {
            const QString op = d->negate ? "NOT LIKE" : "LIKE";
            if (db.driverName() == "QSQLITE" || db.driverName() == "QSQLITE2")
                return d->key + " " + op + " ? ESCAPE '\\'";
            else
                return d->key + " " + op + " ?";
        }
        case None:
            if (d->combine == QDjangoWherePrivate::NoCombine)
            {
                return d->negate ? QString("1 != 0") : QString();
            } else {
                QStringList bits;
                foreach (const QDjangoWhere &child, d->children)
                {
                    QString atom = child.sql(db);
                    if (child.d->children.isEmpty())
                        bits << atom;
                    else
                        bits << QString("(%1)").arg(atom);
                }
                QString combined;
                if (d->combine == QDjangoWherePrivate::AndCombine)
                    combined = bits.join(" AND ");
                else if (d->combine == QDjangoWherePrivate::OrCombine)
                    combined = bits.join(" OR ");
                if (d->negate)
                    combined = QString("NOT (%1)").arg(combined);
                return combined;
            }
    }
    return QString();
}

