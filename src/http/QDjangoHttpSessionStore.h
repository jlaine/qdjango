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

#ifndef QDJANGOHTTPSESSIONSTORE_H
#define QDJANGOHTTPSESSIONSTORE_H

#include <QObject>
#include "QDjangoHttpSession.h"

class QDjangoHttpSessionStorePrivate;
class QDjangoHttpSessionStore : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(QDjangoHttpSessionStore)

    explicit QDjangoHttpSessionStore();

public:
    /**
       Get the instance of application session store
       This method is thread safe.
    */
    static QDjangoHttpSessionStore *instance();

    /**
       Set the max inactive interval, only affect next sessions
       This method is thread safe.
    */
    void setMaxInactiveInterval(qint64 msecs);

    /**
       Get a HTTP session by it's UUID.
       This method is thread safe.
       @return If there is no such session, the function returns a new session with this UUID.
       @param uuid UUID of the session, if not passed a new one will be generated
       @see HttpSession::isNull()
    */
    QDjangoHttpSession newSession(QUuid uuid = QUuid());

    /**
       Get a HTTP session by it's UUID.
       This method is thread safe.
       @return If there is no such session, the function returns a null with null UUID.
       @param uuid UUID of the session
       @see HttpSession::isNull()
    */
    QDjangoHttpSession existingSession(const QUuid& uuid);

private slots:
    void checkInvalidSessions();

private:
    QDjangoHttpSessionStorePrivate* d;
};

#endif // QDJANGOHTTPSESSIONSTORE_H
