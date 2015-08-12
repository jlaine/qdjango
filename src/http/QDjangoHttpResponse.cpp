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

#include "QDjangoHttpResponse.h"
#include "QDjangoHttpResponse_p.h"

/** Constructs a new HTTP response.
 */
QDjangoHttpResponse::QDjangoHttpResponse()
    : d(new QDjangoHttpResponsePrivate)
{
    setHeader(QLatin1String("Content-Length"), QLatin1String("0"));
    setStatusCode(QDjangoHttpResponse::OK);
}

/** Destroys the HTTP response.
 */
QDjangoHttpResponse::~QDjangoHttpResponse()
{
    delete d;
}

/** Returns the raw body of the HTTP response.
 */
QByteArray QDjangoHttpResponse::body() const
{
    return d->body;
}

/** Sets the raw body of the HTTP response.
 *
 * The Content-Length header will be updated to reflect the body size.
 *
 * \param body
 */
void QDjangoHttpResponse::setBody(const QByteArray &body)
{
    d->body = body;
    setHeader(QLatin1String("Content-Length"), QString::number(d->body.size()));
}

/** Returns the specified HTTP response header.
 *
 * \param key
 */
QString QDjangoHttpResponse::header(const QString &key) const
{
    QString lowercaseKey = key.toLower();
    QList<QPair<QString, QString> >::ConstIterator it = d->headers.constBegin();
    while (it != d->headers.constEnd()) {
        if ((*it).first.toLower() == lowercaseKey)
            return (*it).second;
        ++it;
    }
    return QString();
}

/** Sets the specified HTTP response header.
 *
 * \param key
 * \param value
 */
void QDjangoHttpResponse::setHeader(const QString &key, const QString &value)
{
    QString lowercaseKey = key.toLower();
    QList<QPair<QString, QString> >::Iterator it = d->headers.begin();
    while (it != d->headers.end()) {
        if ((*it).first.toLower() == lowercaseKey) {
            (*it).second = value;
            return;
        }
        ++it;
    }
    // not found so add
    d->headers.append(qMakePair(key, value));
}

/** Returns true if the response is ready to be sent.
 *
 * The default implementation always returns true. If you subclass
 * QDjangoHttpResponse to support responses which should only be sent
 * to the client at a later point, you need to reimplement this method
 * and emit the ready() signal once the response is ready.
 */
bool QDjangoHttpResponse::isReady() const
{
    return true;
}

/** Returns the reason for the HTTP response status line.
 */
QString QDjangoHttpResponse::reasonPhrase() const
{
    return d->reasonPhrase;
}

/** Returns the code for the HTTP response status line.
 */
int QDjangoHttpResponse::statusCode() const
{
    return d->statusCode;
}

/** Sets the code for the HTTP response status line.
 *
 * \param code
 */
void QDjangoHttpResponse::setStatusCode(int code)
{
    d->statusCode = code;
    switch(code)
    {
    case OK:
        d->reasonPhrase = QLatin1String("OK");
        break;
    case MovedPermanently:
        d->reasonPhrase = QLatin1String("Moved Permanently");
        break;
    case Found:
        d->reasonPhrase = QLatin1String("Found");
        break;
    case NotModified:
        d->reasonPhrase = QLatin1String("Not Modified");
        break;
    case BadRequest:
        d->reasonPhrase = QLatin1String("Bad Request");
        break;
    case AuthorizationRequired:
        d->reasonPhrase = QLatin1String("Authorization Required");
        break;
    case Forbidden:
        d->reasonPhrase = QLatin1String("Forbidden");
        break;
    case NotFound:
        d->reasonPhrase = QLatin1String("Not Found");
        break;
    case MethodNotAllowed:
        d->reasonPhrase = QLatin1String("Method Not Allowed");
        break;
    case InternalServerError:
        d->reasonPhrase = QLatin1String("Internal Server Error");
        break;
    default:
        d->reasonPhrase = QLatin1String("");
        break;
    }
}

