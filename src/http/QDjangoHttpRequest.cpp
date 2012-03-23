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

#include <QIODevice>

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
