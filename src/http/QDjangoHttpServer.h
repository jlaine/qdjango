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

#ifndef QDJANGO_HTTP_SERVER_H
#define QDJANGO_HTTP_SERVER_H

#include <QHostAddress>
#include <QObject>

#include "QDjangoHttp_p.h"

class QDjangoHttpRequest;
class QDjangoHttpResponse;
class QDjangoHttpServer;
class QDjangoHttpServerPrivate;
class QDjangoUrlResolver;

/** \brief The QDjangoHttpServer class represents an HTTP server.
 *
 *  It allows you to create a standalone HTTP server which will
 *  serve your web application.
 *
 *  To register views, see urls().
 *
 * \ingroup Http
 * \sa QDjangoFastCgiServer
 */
class QDJANGO_EXPORT QDjangoHttpServer : public QObject
{
    Q_OBJECT

public:
    QDjangoHttpServer(QObject *parent = 0);
    ~QDjangoHttpServer();

    void close();
    bool listen(const QHostAddress &address, quint16 port);
    QDjangoUrlResolver *urls() const;

signals:
    /** This signal is emitted when a request completes.
     */
    void requestFinished(QDjangoHttpRequest *request, QDjangoHttpResponse *response);

private slots:
    void _q_newTcpConnection();

private:
    Q_DISABLE_COPY(QDjangoHttpServer)
    QDjangoHttpServerPrivate* const d;
};

#endif
