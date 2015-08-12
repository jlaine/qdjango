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

#ifndef QDJANGOMETAMODEL_H
#define QDJANGOMETAMODEL_H

#include <QMap>
#include <QSharedDataPointer>
#include <QVariant>

#include "QDjango_p.h"

class QDjangoMetaFieldPrivate;
class QDjangoMetaModelPrivate;

/** \brief The QDjangoMetaField class holds the database schema for a field.
 *
 * \internal
 */
class QDJANGO_DB_EXPORT QDjangoMetaField
{
public:
    QDjangoMetaField();
    QDjangoMetaField(const QDjangoMetaField &other);
    ~QDjangoMetaField();
    QDjangoMetaField& operator=(const QDjangoMetaField &other);

    QString column() const;
    bool isAutoIncrement() const;
    bool isBlank() const;
    bool isNullable() const;
    bool isUnique() const;
    bool isValid() const;
    QString name() const;
    int maxLength() const;
    QVariant toDatabase(const QVariant &value) const;

private:
    QSharedDataPointer<QDjangoMetaFieldPrivate> d;
    friend class QDjangoMetaModel;
};

/** \brief The QDjangoMetaModel class holds the database schema for a model.
 *
 *  It manages table creation and deletion operations as well as row
 *  serialisation, deserialisation and deletion operations.
 *
 * \internal
 */
class QDJANGO_DB_EXPORT QDjangoMetaModel
{
public:
    QDjangoMetaModel(const QMetaObject *model = 0);
    QDjangoMetaModel(const QDjangoMetaModel &other);
    ~QDjangoMetaModel();
    QDjangoMetaModel& operator=(const QDjangoMetaModel &other);

    bool isValid() const;

    bool createTable() const;
    QStringList createTableSql() const;
    bool dropTable() const;

    void load(QObject *model, const QVariantList &props, int &pos) const;
    bool remove(QObject *model) const;
    bool save(QObject *model) const;

    QObject *foreignKey(const QObject *model, const char *name) const;
    void setForeignKey(QObject *model, const char *name, QObject *value) const;

    QString className() const;
    QDjangoMetaField localField(const char *name) const;
    QList<QDjangoMetaField> localFields() const;
    QMap<QByteArray, QByteArray> foreignFields() const;
    QByteArray primaryKey() const;
    QString table() const;

private:
    QSharedDataPointer<QDjangoMetaModelPrivate> d;
};

#endif
