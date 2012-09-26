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

#include "shares-models.h"

File::File(QObject *parent)
    : QDjangoModel(parent), m_size(0)
{
}

QDateTime File::date() const
{
    return m_date;
}

void File::setDate(const QDateTime &date)
{
    m_date = date;
}

QByteArray File::hash() const
{
    return m_hash;
}

void File::setHash(const QByteArray &hash)
{
    m_hash = hash;
}

QString File::path() const
{
    return m_path;
}

void File::setPath(const QString &path)
{
    m_path = path;
}

qint64 File::size() const
{
    return m_size;
}

void File::setSize(qint64 size)
{
    m_size = size;
}

