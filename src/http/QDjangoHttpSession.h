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

#include <QDateTime>
#include <QReadWriteLock>
#include <QSharedData>
#include <QUuid>
#include <QVariantMap>

#ifndef QDJANGOHTTPSESSION_H
#define QDJANGOHTTPSESSION_H

class QDjangoHttpSessionData : public QSharedData {
public:

    /** Flag to be used to invalidate the session */
    volatile bool isValid;

    /** Timestamp of creation time */
    QDateTime creation;

    /** Timestamp of last access, renewed by the HttpSessionStore */
    QDateTime lastAccess;

    /** Time interval maximun for invalidate session, set by the HttpSessionStore */
    qint64 maxInactiveInterval;

    /** Storage for the key/value pairs; */
    QVariantMap values;

    /** Used to synchronize threads */
    QReadWriteLock lock;
};

class QDjangoHttpSession
{
    friend class QDjangoHttpSessionStore;

private:
    /**
      Creates a new and valid QDjangoHttpSession object if uuid is not null
    */
    QDjangoHttpSession(QUuid uuid, qint64 maxInactiveInterval);

public:
    /**
      Creates an invalid QDjangoHttpSession object
    */
    QDjangoHttpSession();

    /**
      Copy constructor. Creates another QDjangoHttpSession object that shares the
      data of the other object.
    */
    QDjangoHttpSession(const QDjangoHttpSession& other);

    /**
      Copy operator. Detaches from the current shared data and attaches to
      the data of the other object.
    */
    QDjangoHttpSession& operator= (const QDjangoHttpSession& other);

    /** The session are recently created. This method is thread safe. */
    bool isNew() const;

    /** Not Valid sessions cannot store data. This method is thread safe. */
    bool isValid() const;

    /** Restrieve the session unique ID */
    QUuid uuid() const;

    /**
      Get the timestamp of creation of this session.
      This method is thread safe.
    */
    QDateTime creation() const;

    /**
      Get the timestamp of last access. That is the time when the last
      HttpSessionStore::getSession() has been called.
      This method is thread safe.
    */
    QDateTime lastAccess() const;

    /** Set a value. This method is thread safe. */
    bool setValue(const QString& key, const QVariant& value);

    /** Add a value. This method is thread safe. */
    bool addValue(const QString& key, const QVariant& value);

    /** Remove a value. This method is thread safe. */
    bool removeValue(const QString& key);

    /** Get a value. This method is thread safe. */
    QVariant value(const QString& key, const QVariant& defaultValue = QVariant()) const;

    /** Get a values. This method is thread safe. */
    QVariantList values(const QString& key) const;

    /** Check if a key exists. This method is thread safe. */
    bool contains(const QString& key) const;

    /**
      Get a copy of all data stored in this session.
      Changes to the session do not affect the copy and vice versa.
      This method is thread safe.
    */
    QVariantMap allValues() const;

    /** Invalidate a session, can be used on logout process */
    void invalidate();

    /**
      Set the timestamp of last access, to renew the timeout period.
      Called by  HttpSessionStore::getSession().
      This method is thread safe.
    */
    bool renew();

private:
    /** Unique ID */
    QUuid m_uuid;

    /** Pointer to the shared data. */
    QExplicitlySharedDataPointer<QDjangoHttpSessionData> m_d;
};

#endif // QDJANGOHTTPSESSION_H
