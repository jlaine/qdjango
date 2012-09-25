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

#ifndef QDJANGO_HTTP_REQUEST_H
#define QDJANGO_HTTP_REQUEST_H

#include <QString>

#include "QDjangoHttp_p.h"

class QDjangoHttpRequestPrivate;

/** \defgroup Http
 *
 * QDjango's HTTP request and response framework enables you to write web
 * web applications and serve them over HTTP.
 */

/** \brief The QDjangoHttpRequest class represents an HTTP request.
 *
 * \ingroup Http
 */
class QDJANGO_EXPORT QDjangoHttpRequest
{
public:
    QDjangoHttpRequest();
    ~QDjangoHttpRequest();

    QByteArray body() const;
    QString get(const QString &key) const;
    QString meta(const QString &key) const;
    QString method() const;
    QString path() const;
    QString post(const QString &key) const;

private:
    Q_DISABLE_COPY(QDjangoHttpRequest)
    QDjangoHttpRequestPrivate* const d;
    friend class QDjangoFastCgiConnection;
    friend class QDjangoHttpConnection;
    friend class QDjangoHttpTestRequest;
};

/** \cond */

class QDJANGO_EXPORT QDjangoHttpTestRequest : public QDjangoHttpRequest
{
public:
    QDjangoHttpTestRequest(const QString &method, const QString &path);

private:
    Q_DISABLE_COPY(QDjangoHttpTestRequest)
};

/** \endcond */

#endif
