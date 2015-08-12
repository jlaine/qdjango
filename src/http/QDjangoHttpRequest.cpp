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

#include <QIODevice>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QUrlQuery>
#else
#include <QUrl>
#endif

#include "QDjangoHttpRequest.h"
#include "QDjangoHttpRequest_p.h"

/** Constructs a new HTTP request.
 */
QDjangoHttpRequest::QDjangoHttpRequest()
    : d(new QDjangoHttpRequestPrivate)
{
}

/** Destroys the HTTP request.
 */
QDjangoHttpRequest::~QDjangoHttpRequest()
{
    delete d;
}

/** Returns the raw body of the HTTP request.
 */
QByteArray QDjangoHttpRequest::body() const
{
    return d->buffer;
}

/** Returns the GET data for the given \a key.
 */
QString QDjangoHttpRequest::get(const QString &key) const
{
    QString queryString = d->meta.value(QLatin1String("QUERY_STRING"));
    queryString.replace('+', ' ');
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    QUrlQuery query(queryString);
    return query.queryItemValue(key, QUrl::FullyDecoded);
#else
    QUrl url;
    url.setEncodedQuery(queryString.toLatin1());
    return url.queryItemValue(key);
#endif
}

/** Returns the specified HTTP request header.
 *
 * \param key
 */
QString QDjangoHttpRequest::meta(const QString &key) const
{
    return d->meta.value(key);
}

/** Returns the HTTP request's method (e.g. GET, POST).
 */
QString QDjangoHttpRequest::method() const
{
    return d->method;
}

/** Returns the HTTP request's path.
 */
QString QDjangoHttpRequest::path() const
{
    return d->path;
}

/** Returns the POST data for the given \a key.
 */
QString QDjangoHttpRequest::post(const QString &key) const
{
    QByteArray buffer = d->buffer;
    buffer.replace('+', ' ');
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    QUrlQuery query(QString::fromUtf8(buffer));
    return query.queryItemValue(key, QUrl::FullyDecoded);
#else
    QUrl url;
    url.setEncodedQuery(buffer);
    return url.queryItemValue(key);
#endif
}

QDjangoHttpTestRequest::QDjangoHttpTestRequest(const QString &method, const QString &path)
{
    d->method = method;
    d->path = path;
}
