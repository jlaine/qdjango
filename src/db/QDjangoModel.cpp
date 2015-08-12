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

#include <QDebug>
#include <QStringList>

#include "QDjango.h"
#include "QDjangoModel.h"
#include "QDjangoQuerySet.h"

/** Construct a new QDjangoModel.
 *
 * \param parent
 */
QDjangoModel::QDjangoModel(QObject *parent)
    : QObject(parent)
{
}

/** Returns the primary key for this QDjangoModel.
 */
QVariant QDjangoModel::pk() const
{
    const QDjangoMetaModel metaModel = QDjango::metaModel(metaObject()->className());
    return property(metaModel.primaryKey());
}

/** Sets the primary key for this QDjangoModel.
 *
 * \param pk
 */
void QDjangoModel::setPk(const QVariant &pk)
{
    const QDjangoMetaModel metaModel = QDjango::metaModel(metaObject()->className());
    setProperty(metaModel.primaryKey(), pk);
}

/** Retrieves the QDjangoModel pointed to by the given foreign-key.
 *
 * \param name
 */
QObject *QDjangoModel::foreignKey(const char *name) const
{
    const QDjangoMetaModel metaModel = QDjango::metaModel(metaObject()->className());
    return metaModel.foreignKey(this, name);
}

/** Sets the QDjangoModel pointed to by the given foreign-key.
 *
 * \param name
 * \param value
 *
 * \note The QDjangoModel will not take ownership of the given \c value.
 */
void QDjangoModel::setForeignKey(const char *name, QObject *value)
{
    const QDjangoMetaModel metaModel = QDjango::metaModel(metaObject()->className());
    metaModel.setForeignKey(this, name, value);
}

/** Deletes the QDjangoModel from the database.
 *
 * \return true if deletion succeeded, false otherwise
 */
bool QDjangoModel::remove()
{
    const QDjangoMetaModel metaModel = QDjango::metaModel(metaObject()->className());
    return metaModel.remove(this);
}

/** Saves the QDjangoModel to the database.
 *
 * \return true if saving succeeded, false otherwise
 */
bool QDjangoModel::save()
{
    const QDjangoMetaModel metaModel = QDjango::metaModel(metaObject()->className());
    return metaModel.save(this);
}

/** Returns a string representation of the model instance.
 */
QString QDjangoModel::toString() const
{
    const QDjangoMetaModel metaModel = QDjango::metaModel(metaObject()->className());
    const QByteArray pkName = metaModel.primaryKey();
    return QString::fromLatin1("%1(%2=%3)").arg(QString::fromLatin1(metaObject()->className()), QString::fromLatin1(pkName), property(pkName).toString());
}

