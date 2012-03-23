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

#ifndef QDJANGO_SHARES_MODELS_H
#define QDJANGO_SHARES_MODELS_H

#include <QDateTime>

#include "QDjangoModel.h"

class File : public QDjangoModel
{
    Q_OBJECT
    Q_PROPERTY(QDateTime date READ date WRITE setDate)
    Q_PROPERTY(QByteArray hash READ hash WRITE setHash)
    Q_PROPERTY(QString path READ path WRITE setPath)
    Q_PROPERTY(qint64 size READ size WRITE setSize)

    Q_CLASSINFO("path", "max_length=255 primary_key=true")
    Q_CLASSINFO("hash", "max_length=32")

public:
    File(QObject *parent = 0);

    QDateTime date() const;
    void setDate(const QDateTime &date);

    QByteArray hash() const;
    void setHash(const QByteArray &hash);

    QString path() const;
    void setPath(const QString &path);

    qint64 size() const;
    void setSize(qint64 size);

private:
    QDateTime m_date;
    QByteArray m_hash;
    QString m_path;
    qint64 m_size;
};

#endif
