/*
 * Copyright (C) 2010-2014 Israel Lins
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

#include "QDjangoHttpSession.h"

QDjangoHttpSession::QDjangoHttpSession(QUuid uuid, qint64 maxInactiveInterval) :
    m_uuid(uuid)
{
    if (!m_uuid.isNull()) {
        m_d = new QDjangoHttpSessionData();
        m_d->isValid = true;
        m_d->creation = m_d->lastAccess = QDateTime::currentDateTime();
        m_d->maxInactiveInterval = maxInactiveInterval;
    }
}

QDjangoHttpSession::QDjangoHttpSession()
{}

QDjangoHttpSession::QDjangoHttpSession(const QDjangoHttpSession &other)
{
    m_uuid = other.m_uuid;
    m_d = other.m_d;
}

QDjangoHttpSession &QDjangoHttpSession::operator=(const QDjangoHttpSession &other)
{
    m_uuid = other.m_uuid;
    m_d = other.m_d;
    return *this;
}

bool QDjangoHttpSession::isNew() const
{
    bool r = false;
    if (m_d) {
        m_d->lock.lockForRead();
        r = m_d->creation == m_d->lastAccess;
        m_d->lock.unlock();
    }
    return r;
}

bool QDjangoHttpSession::isValid() const
{
    bool r = m_d;
    if (m_d) {
        m_d->lock.lockForRead();
        r = m_d->isValid && ((m_d->maxInactiveInterval <= 0) ||
            (m_d->maxInactiveInterval > QDateTime::currentDateTime().msecsTo(m_d->lastAccess)));
        m_d->lock.unlock();
    }
    return r;
}

QUuid QDjangoHttpSession::uuid() const
{
    return m_uuid;
}

QDateTime QDjangoHttpSession::creation() const
{
    QDateTime r;
    if (m_d) {
        m_d->lock.lockForRead();
        r = m_d->creation;
        m_d->lock.unlock();
    }
    return r;
}

QDateTime QDjangoHttpSession::lastAccess() const
{
    QDateTime r;
    if (m_d) {
        m_d->lock.lockForRead();
        r = m_d->lastAccess;
        m_d->lock.unlock();
    }
    return r;
}

bool QDjangoHttpSession::setValue(const QString &key, const QVariant &value)
{
    if (isValid()) {
        m_d->lock.lockForWrite();
        m_d->values.insert(key, value);
        m_d->lock.unlock();
        return true;
    }
    return false;
}

bool QDjangoHttpSession::addValue(const QString &key, const QVariant &value)
{
    if (isValid()) {
        m_d->lock.lockForWrite();
        m_d->values.insertMulti(key, value);
        m_d->lock.unlock();
        return true;
    }
    return false;
}

bool QDjangoHttpSession::removeValue(const QString &key)
{
    bool r = false;
    if (isValid()) {
        m_d->lock.lockForWrite();
        r = (m_d->values.remove(key) > 0);
        m_d->lock.unlock();
    }
    return r;
}

QVariant QDjangoHttpSession::value(const QString &key, const QVariant &defaultValue) const
{
    QVariant r = defaultValue;
    if (m_d) {
        m_d->lock.lockForRead();
        r = m_d->values.value(key);
        m_d->lock.unlock();
    }
    return r;
}

QVariantList QDjangoHttpSession::values(const QString &key) const
{
    QVariantList r;
    if (m_d) {
        m_d->lock.lockForRead();
        r = m_d->values.values(key);
        m_d->lock.unlock();
    }
    return r;
}

bool QDjangoHttpSession::contains(const QString &key) const
{
    bool r = false;
    if (m_d) {
        m_d->lock.lockForWrite();
        r = m_d->values.contains(key);
        m_d->lock.unlock();
    }
    return r;
}

QVariantMap QDjangoHttpSession::allValues() const
{
    QVariantMap r;
    if (m_d) {
        m_d->lock.lockForRead();
        r = m_d->values;
        m_d->lock.unlock();
    }
    return r;
}

void QDjangoHttpSession::invalidate()
{
    if (m_d) {
        m_d->lock.lockForWrite();
        m_d->isValid = false;
        m_d->lock.unlock();
    }
}

bool QDjangoHttpSession::renew()
{
    if (isValid()) {
        m_d->lock.lockForWrite();
        m_d->lastAccess = QDateTime::currentDateTime();
        m_d->lock.unlock();
        return true;
    }
    return false;
}
