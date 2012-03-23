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

#ifndef QDJANGO_P_H
#define QDJANGO_P_H

#include <QDebug>
#include <QDateTime>
#include <QMap>
#include <QMutex>
#include <QObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

/** \brief The QDjangoMetaField class holds the database schema for a field.
 *
 * \internal
 */
class QDjangoMetaField
{
public:
    QDjangoMetaField();

    QByteArray name;
    QVariant::Type type;
    bool autoIncrement;
    bool index;
    int maxLength;
    bool unique;
    QString foreignModel;
};

/** \brief The QDjangoMetaModel class holds the database schema for a model.
 *
 *  It manages table creation and deletion operations as well as row
 *  serialisation, deserialisation and deletion operations.
 *
 * \internal
 */
class QDjangoMetaModel
{
public:
    QDjangoMetaModel(const QObject *model = 0);
    bool isValid() const;

    bool createTable() const;
    bool dropTable() const;

    void load(QObject *model, const QVariantList &props, int &pos) const;
    bool remove(QObject *model) const;
    bool save(QObject *model) const;

    QObject *foreignKey(const QObject *model, const char *name) const;
    void setForeignKey(QObject *model, const char *name, QObject *value) const;

    QList<QDjangoMetaField> localFields() const;
    QMap<QByteArray, QString> foreignFields() const;
    QByteArray primaryKey() const;
    QString table() const;

private:
    QList<QDjangoMetaField> m_localFields;
    QMap<QByteArray, QString> m_foreignFields;
    QByteArray m_primaryKey;
    QString m_table;
};

/** \brief The QDjangoDatabase class represents a set of connections to a
 *  database.
 *
 * \internal
 */
class QDjangoDatabase : public QObject
{
    Q_OBJECT

public:
    QDjangoDatabase(QObject *parent = 0);

    QSqlDatabase reference;
    QMutex mutex;
    QMap<QThread*, QSqlDatabase> copies;
    qint64 connectionId;

private slots:
    void threadFinished();
};

class QDjangoQuery : public QSqlQuery
{
public:
    QDjangoQuery(QSqlDatabase db) : QSqlQuery(db)
    {
    }

    void addBindValue(const QVariant &val, QSql::ParamType paramType = QSql::In)
    {
        // this hack is required so that we do not store a mix of local
        // and UTC times
        if (val.type() == QVariant::DateTime)
            QSqlQuery::addBindValue(val.toDateTime().toLocalTime(), paramType);
        else
            QSqlQuery::addBindValue(val, paramType);
    }

#ifdef QDJANGO_DEBUG_SQL
    bool exec()
    {
        qDebug() << "SQL query" << lastQuery();
        QMapIterator<QString, QVariant> i(boundValues());
        while (i.hasNext()) {
            i.next();
            qDebug() << "SQL   " << i.key().toAscii().data() << "="
                     << i.value().toString().toAscii().data();
        }
        if (!QSqlQuery::exec()) {
            qWarning() << "SQL error" << lastError();
            return false;
        }
        return true;
    }

    bool exec(const QString &query)
    {
        qDebug() << "SQL query" << query;
        if (!QSqlQuery::exec(query)) {
            qWarning() << "SQL error" << lastError();
            return false;
        }
        return true;
    }
#endif
};

#endif
