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

#include "QDjangoHttpSessionStore.h"

#include <QCoreApplication>
#include <QHash>
#include <QMutex>
#include <QMutexLocker>
#include <QTimer>

class QDjangoHttpSessionStorePrivate {
public:
    QHash<QUuid, QDjangoHttpSession> sessions;
    qint64 maxInativeInterval;
    QMutex sessionMutex;
    QTimer checkInvalidSessions;
};

QDjangoHttpSessionStore::QDjangoHttpSessionStore():
    QObject()
{
    d = new QDjangoHttpSessionStorePrivate;
    d->maxInativeInterval = 30 * 60 * 1000; // 30 minutes
    d->checkInvalidSessions.setInterval(60000);
    connect(&d->checkInvalidSessions, SIGNAL(timeout()), SLOT(checkInvalidSessions()));
}

QDjangoHttpSessionStore *QDjangoHttpSessionStore::instance()
{
    static QDjangoHttpSessionStore store;
    return &store;
}

void QDjangoHttpSessionStore::setMaxInactiveInterval(qint64 msecs)
{
    d->maxInativeInterval = msecs;
}

QDjangoHttpSession QDjangoHttpSessionStore::newSession(QUuid uuid)
{
    QDjangoHttpSession session;
    QMutexLocker locker(&(d->sessionMutex));

    if (uuid.isNull()) {
        do {
            uuid = QUuid::createUuid();
        } while (d->sessions.contains(uuid));
    } else if (d->sessions.contains(uuid)) {
        session = d->sessions.value(uuid);
        session.renew();
    }

    if (!session.isValid()) {
        session = QDjangoHttpSession(uuid, d->maxInativeInterval);
        d->sessions.insert(uuid, session);
    }

    return session;
}

QDjangoHttpSession QDjangoHttpSessionStore::existingSession(const QUuid &uuid)
{
    QDjangoHttpSession session;
    QMutexLocker locker(&(d->sessionMutex));

    if (d->sessions.contains(uuid)) {
        session = d->sessions.value(uuid);
        session.renew();
    }

    return session;
}

void QDjangoHttpSessionStore::checkInvalidSessions()
{
    QMutexLocker locker(&(d->sessionMutex));
    foreach (QDjangoHttpSession session, d->sessions) {
        if (!session.isValid())
            d->sessions.remove(session.uuid());
    }
}
