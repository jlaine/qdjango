/*
 * Copyright (C) 2010-2012 Jeremy Lainé
 * Contact: http://code.google.com/p/qdjango/
 *
 * This file is part of the QDjango Library.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "QDjangoHttpResponse.h"
#include "QDjangoHttpResponse_p.h"

/** Constructs a new HTTP response.
 */
QDjangoHttpResponse::QDjangoHttpResponse()
    : d(new QDjangoHttpResponsePrivate)
{
    setHeader("Content-Length", "0");
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
    setHeader("Content-Length", QString::number(d->body.size()));
}

/** Returns the specified HTTP response header.
 *
 * \param key
 */
QString QDjangoHttpResponse::header(const QString &key) const
{
    return d->header.value(key);
}

/** Sets the specified HTTP response header.
 *
 * \param key
 * \param value
 */
void QDjangoHttpResponse::setHeader(const QString &key, const QString &value)
{
    d->header.setValue(key, value);
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

/** Returns the code for the HTTP response status line.
 */
int QDjangoHttpResponse::statusCode() const
{
    return d->header.statusCode();
}

/** Sets the code for the HTTP response status line.
 *
 * \param code
 */
void QDjangoHttpResponse::setStatusCode(int code)
{
    switch(code)
    {
    case OK:
        d->header.setStatusLine(code, "OK");
        break;
    case MovedPermanently:
        d->header.setStatusLine(code, "Moved Permanently");
        break;
    case Found:
        d->header.setStatusLine(code, "Found");
        break;
    case NotModified:
        d->header.setStatusLine(code, "Not Modified");
        break;
    case BadRequest:
        d->header.setStatusLine(code, "Bad Request");
        break;
    case AuthorizationRequired:
        d->header.setStatusLine(code, "Authorization Required");
        break;
    case Forbidden:
        d->header.setStatusLine(code, "Forbidden");
        break;
    case NotFound:
        d->header.setStatusLine(code, "Not Found");
        break;
    case MethodNotAllowed:
        d->header.setStatusLine(code, "Method Not Allowed");
        break;
    case InternalServerError:
        d->header.setStatusLine(code, "Internal Server Error");
        break;
    default:
        d->header.setStatusLine(code);
        break;
    }
}

