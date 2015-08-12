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

#ifndef QDJANGO_FASTCGI_SERVER_H
#define QDJANGO_FASTCGI_SERVER_H

#include <QHostAddress>
#include <QObject>

#include "QDjangoHttp_p.h"

class QDjangoFastCgiServerPrivate;
class QDjangoHttpController;
class QDjangoUrlResolver;

/** \brief The QDjangoFastCgiServer class represents a FastCGI server.
 *
 *  It allows you to create a FastCGI server which your reverse proxy
 *  (e.g. apache, nginx) will query to serve your web application.
 *
 *  To register views, see urls().
 *
 * \ingroup Http
 * \sa QDjangoHttpServer
 */
class QDJANGO_HTTP_EXPORT QDjangoFastCgiServer : public QObject
{
    Q_OBJECT

public:
    QDjangoFastCgiServer(QObject *parent = 0);
    ~QDjangoFastCgiServer();

    void close();
    bool listen(const QString &name);
    bool listen(const QHostAddress &address, quint16 port);
    QDjangoUrlResolver *urls() const;

private slots:
    void _q_newLocalConnection();
    void _q_newTcpConnection();

private:
    Q_DISABLE_COPY(QDjangoFastCgiServer)
    QDjangoFastCgiServerPrivate *d;
};

#endif
