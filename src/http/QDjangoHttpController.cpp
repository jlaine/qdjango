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

#include <QCoreApplication>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QLocale>
#include <QRegExp>
#include <QStringList>
#include <QUrl>

#include "QDjangoHttpController.h"
#include "QDjangoHttpRequest.h"
#include "QDjangoHttpResponse.h"

/** Extract basic credentials from an HTTP \a request.
 *
 * Returns \b true if credentials were provider, \b false otherwise.
 */
bool QDjangoHttpController::getBasicAuth(const QDjangoHttpRequest &request, QString &username, QString &password)
{
    QRegExp authRx(QLatin1String("^Basic (.+)$"));
    const QString authHeader = request.meta(QLatin1String("HTTP_AUTHORIZATION"));
    if (authRx.exactMatch(authHeader)) {
        const QString authValue = QString::fromUtf8(QByteArray::fromBase64(authRx.cap(1).toLatin1()));
        const QStringList bits = authValue.split(QLatin1Char(':'));
        if (bits.size() == 2 && !bits[0].isEmpty() && !bits[1].isEmpty()) {
            username = bits[0];
            password = bits[1];
            return true;
        }
    }
    return false;
}

/** Converts a QDateTime to an HTTP datetime string.
 */
QString QDjangoHttpController::httpDateTime(const QDateTime &dt)
{
    if (dt.isValid())
        return QLocale("C").toString(dt.toUTC(), QLatin1String("ddd, dd MMM yyyy HH:mm:ss")) + QLatin1String(" GMT");
    return QString();
}

/** Converts an HTTP datetime string to a QDateTime.
 */
QDateTime QDjangoHttpController::httpDateTime(const QString &str)
{
    QDateTime dt = QLocale("C").toDateTime(str.left(25), QLatin1String("ddd, dd MMM yyyy HH:mm:ss"));
    dt.setTimeSpec(Qt::UTC);
    return dt;
}

QDjangoHttpResponse *QDjangoHttpController::serveError(const QDjangoHttpRequest &request, int code, const QString &text)
{
    Q_UNUSED(request);

    QDjangoHttpResponse *response = new QDjangoHttpResponse;
    response->setHeader(QLatin1String("Content-Type"), QLatin1String("text/html; charset=utf-8"));
    response->setStatusCode(code);
    response->setBody(QString::fromLatin1("<html>"
        "<head><title>Error</title></head>"
        "<body><p>%1</p></body>"
        "</html>").arg(text).toUtf8());
    return response;
}

/** Respond to an HTTP \a request with an authorization error.
 *
 * \param request
 * \param realm
 */
QDjangoHttpResponse *QDjangoHttpController::serveAuthorizationRequired(const QDjangoHttpRequest &request, const QString &realm)
{
    Q_UNUSED(request);

    QDjangoHttpResponse *response = new QDjangoHttpResponse;
    response->setStatusCode(QDjangoHttpResponse::AuthorizationRequired);
    response->setHeader(QLatin1String("WWW-Authenticate"), QString::fromLatin1("Basic realm=\"%1\"").arg(realm));
    return response;
}

/** Respond to a malformed HTTP request.
 *
 * \param request
 */
QDjangoHttpResponse *QDjangoHttpController::serveBadRequest(const QDjangoHttpRequest &request)
{
    return serveError(request, QDjangoHttpResponse::BadRequest, QLatin1String("Your browser sent a malformed request."));
}

/** Respond to an HTTP \a request with an internal server error.
 *
 * \param request
 */
QDjangoHttpResponse *QDjangoHttpController::serveInternalServerError(const QDjangoHttpRequest &request)
{
    return serveError(request, QDjangoHttpResponse::InternalServerError, QLatin1String("An internal server error was encountered."));
}

/** Respond to an HTTP \a request with a not found error.
 *
 * \param request
 */
QDjangoHttpResponse *QDjangoHttpController::serveNotFound(const QDjangoHttpRequest &request)
{
    return serveError(request, QDjangoHttpResponse::NotFound, QLatin1String("The document you requested was not found."));
}

/** Respond to an HTTP \a request with a redirect.
 *
 * \param request
 * \param url The URL to which the user is redirected.
 * \param permanent Whether the redirect is permanent.
 */
QDjangoHttpResponse *QDjangoHttpController::serveRedirect(const QDjangoHttpRequest &request, const QUrl &url, bool permanent)
{
    const QString urlString = url.toString();
    QDjangoHttpResponse *response = serveError(request, permanent ? QDjangoHttpResponse::MovedPermanently : QDjangoHttpResponse::Found,
        QString::fromLatin1("You are being redirect to <a href=\"%1\">%2</a>").arg(urlString, urlString));
    response->setHeader(QLatin1String("Location"), urlString);
    return response;
}

/** Respond to an HTTP \a request for a static file.
 *
 * \param request
 * \param docPath The path to the document, such that it can be opened using a QFile.
 * \param expires An optional expiry date.
 */
QDjangoHttpResponse *QDjangoHttpController::serveStatic(const QDjangoHttpRequest &request, const QString &docPath, const QDateTime &expires)
{
    QFileInfo info(docPath);
    if (!info.isFile())
        return serveNotFound(request);
    const QString fileName = info.fileName();

    QDjangoHttpResponse *response = new QDjangoHttpResponse;
    response->setStatusCode(QDjangoHttpResponse::OK);

    // determine last modified date
    QDateTime lastModified = info.lastModified();
    if (docPath.startsWith(QLatin1String(":/")))
        lastModified = QFileInfo(qApp->applicationFilePath()).lastModified();
    if (lastModified.isValid())
        response->setHeader(QLatin1String("Last-Modified"), httpDateTime(lastModified));

    // cache expiry
    if (expires.isValid())
        response->setHeader(QLatin1String("Expires"), httpDateTime(expires));

    // handle if-modified-since
    const QDateTime ifModifiedSince = httpDateTime(request.meta(QLatin1String("HTTP_IF_MODIFIED_SINCE")));
    if (lastModified.isValid() && ifModifiedSince.isValid() && lastModified <= ifModifiedSince)
    {
        response->setStatusCode(304);
        return response;
    }

    // determine content type
    QString mimeType;
    if (fileName.endsWith(QLatin1String(".css")))
        mimeType = QLatin1String("text/css");
    else if (fileName.endsWith(QLatin1String(".html")))
        mimeType = QLatin1String("text/html");
    else if (fileName.endsWith(QLatin1String(".js")))
        mimeType = QLatin1String("application/javascript");
    else
        mimeType = QLatin1String("application/octet-stream");
    response->setHeader(QLatin1String("Content-Type"), mimeType);

    // read contents
    QFile file(docPath);
    if (!file.open(QIODevice::ReadOnly)) {
        delete response;
        return serveInternalServerError(request);
    }
    if (request.method() == QLatin1String("HEAD"))
        response->setHeader(QLatin1String("Content-Length"), QString::number(file.size()));
    else
        response->setBody(file.readAll());
    return response;
}

