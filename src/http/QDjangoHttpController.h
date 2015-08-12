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

#ifndef QDJANGO_HTTP_CONTROLLER_H
#define QDJANGO_HTTP_CONTROLLER_H

#include <QDateTime>
#include <QString>

#include "QDjangoHttp_p.h"

class QDjangoHttpRequest;
class QDjangoHttpResponse;
class QUrl;

/** \brief The QDjangoHttpController class provides static methods for replying to HTTP requests.
 *
 * \ingroup Http
 */
class QDJANGO_HTTP_EXPORT QDjangoHttpController
{
public:
    // get basic authorization credentials
    static bool getBasicAuth(const QDjangoHttpRequest &request, QString &username, QString &password);

    // date / time handling
    static QString httpDateTime(const QDateTime &dt);
    static QDateTime httpDateTime(const QString &str);

    // common responses
    static QDjangoHttpResponse *serveAuthorizationRequired(const QDjangoHttpRequest &request, const QString &realm = QLatin1String("Secure Area"));
    static QDjangoHttpResponse *serveBadRequest(const QDjangoHttpRequest &request);
    static QDjangoHttpResponse *serveInternalServerError(const QDjangoHttpRequest &request);
    static QDjangoHttpResponse *serveNotFound(const QDjangoHttpRequest &request);
    static QDjangoHttpResponse *serveRedirect(const QDjangoHttpRequest &request, const QUrl &url, bool permanent = false);
    static QDjangoHttpResponse *serveStatic(const QDjangoHttpRequest &request, const QString &filePath, const QDateTime &expires = QDateTime());

private:
    static QDjangoHttpResponse *serveError(const QDjangoHttpRequest &request, int code, const QString &text);
};

#endif
