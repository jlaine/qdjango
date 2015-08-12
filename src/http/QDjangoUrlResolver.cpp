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

#include <QMetaMethod>
#include <QMetaObject>
#include <QRegExp>
#include <QStringList>

#include "QDjangoHttpController.h"
#include "QDjangoHttpRequest.h"
#include "QDjangoHttpResponse.h"
#include "QDjangoUrlResolver.h"

class QDjangoUrlResolverRoute {
public:
    QDjangoUrlResolverRoute()
        : receiver(0)
        , urls(0)
    {
    }

    QRegExp path;
    QObject *receiver;
    QByteArray member;
    QDjangoUrlResolver *urls;
};

class QDjangoUrlResolverPrivate
{
public:
    QDjangoHttpResponse* respond(const QDjangoHttpRequest &request, const QString &path) const;
    QString reverse(QObject *receiver, const char *member, const QVariantList &args = QVariantList()) const;

    QList<QDjangoUrlResolverRoute> routes;
};

QDjangoHttpResponse* QDjangoUrlResolverPrivate::respond(const QDjangoHttpRequest &request, const QString &path) const
{
    QList<QDjangoUrlResolverRoute>::const_iterator it;
    for (it = routes.constBegin(); it != routes.constEnd(); ++it) {
        if (it->urls && it->path.indexIn(path) == 0) {
            // try recursing
            QString subPath = path.mid(it->path.capturedTexts().first().size());
            QDjangoHttpResponse *response = it->urls->d->respond(request, subPath);
            if (response)
                return response;
        } else if (it->receiver && it->path.exactMatch(path)) {
            // collect arguments
            QStringList caps = it->path.capturedTexts();
            caps.takeFirst();
            QList<QGenericArgument> args;
            args << Q_ARG(QDjangoHttpRequest, request);
            for (int i = 0; i < caps.size(); ++i) {
                args << Q_ARG(QString, caps[i]);
            }
            while (args.size() < 10) {
                args << QGenericArgument();
            }

            QDjangoHttpResponse *response = 0;
            if (!QMetaObject::invokeMethod(it->receiver, it->member.constData(), Qt::DirectConnection,
                    Q_RETURN_ARG(QDjangoHttpResponse*, response),
                    args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9])
                || !response) {
                return QDjangoHttpController::serveInternalServerError(request);
            }
            return response;
        }
    }
    return 0;
}

QString QDjangoUrlResolverPrivate::reverse(QObject *receiver, const char *member, const QVariantList &args) const
{
    QList<QDjangoUrlResolverRoute>::const_iterator it;
    for (it = routes.constBegin(); it != routes.constEnd(); ++it) {
        // recurse
        if (it->urls) {
            QString path = it->urls->d->reverse(receiver, member, args);
            if (!path.isNull()) {
                QString prefix = it->path.pattern();
                if (prefix.startsWith(QLatin1Char('^')))
                    prefix.remove(0, 1);
                if (prefix.endsWith(QLatin1Char('$')))
                    prefix.chop(1);

                return prefix + path;
            }
        } else if (it->receiver == receiver && it->member == member) {
            QString path = it->path.pattern();
            if (path.startsWith(QLatin1Char('^')))
                path.remove(0, 1);
            if (path.endsWith(QLatin1Char('$')))
                path.chop(1);

            // replace parameters
            QVariantList arguments(args);
            int pos = 0;
            QRegExp rx(QLatin1String("\\([^)]+\\)"));
            while ((pos = rx.indexIn(path, pos)) != -1) {
                if (arguments.isEmpty()) {
                    qWarning("Too few arguments for '%s'", member);
                    return QString();
                }
                const QString str = arguments.takeFirst().toString();
                path.replace(pos, rx.matchedLength(), str);
                pos += str.size();
            }
            if (!arguments.isEmpty()) {
                qWarning("Too many arguments for '%s'", member);
                return QString();
            }
            if (path.isEmpty())
                return QLatin1String("");
            else
                return path;
        }
    }

    // not found
    return QString();
}

/** Constructs a new URL resolver with the given \a parent.
 */
QDjangoUrlResolver::QDjangoUrlResolver(QObject *parent)
    : QObject(parent)
    , d(new QDjangoUrlResolverPrivate)
{
}

QDjangoUrlResolver::~QDjangoUrlResolver()
{
    delete d;
}

/** Adds a URL mapping for the given \a path.
 */
bool QDjangoUrlResolver::set(const QRegExp &path, QObject *receiver, const char *member)
{
    Q_ASSERT(receiver);
    Q_ASSERT(member);

    const QMetaObject *metaObject = receiver->metaObject();
    QByteArray needle(member);
    needle += '(';
    for (int i = metaObject->methodOffset(); i < metaObject->methodCount(); ++i) {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
        if (metaObject->method(i).name() == member) {
#else
        const QByteArray signature = metaObject->method(i).signature();
        if (signature.startsWith(needle)) {
#endif

            // check parameter types
            const QList<QByteArray> ptypes = metaObject->method(i).parameterTypes();
            if (ptypes.isEmpty() || ptypes[0] != "QDjangoHttpRequest") {
                qWarning("First argument of '%s' should be a QDjangoHttpRequest", member);
                return false;
            }

            // register route
            QDjangoUrlResolverRoute route;
            route.path = path;
            route.receiver = receiver;
            route.member = member;
            d->routes << route;
            return true;
        }
    }

    qWarning("Could not find '%s' in receiver", member);
    return false;
}

/** Adds a URL mapping for the given \a path.
 */
bool QDjangoUrlResolver::include(const QRegExp &path, QDjangoUrlResolver *urls)
{
    Q_ASSERT(urls);

    // register route
    QDjangoUrlResolverRoute route;
    route.path = path;
    route.urls = urls;
    d->routes << route;
    return true;
}

/** Responds to the given HTTP \a request for the given \a path.
 */
QDjangoHttpResponse* QDjangoUrlResolver::respond(const QDjangoHttpRequest &request, const QString &path) const
{
    QString fixedPath(path);
    if (fixedPath.startsWith(QLatin1Char('/')))
        fixedPath.remove(0, 1);

    QDjangoHttpResponse *response = d->respond(request, fixedPath);
    if (response)
        return response;
    else
        return QDjangoHttpController::serveNotFound(request);
}

/** Returns the URL for the member \a member of \a receiver with
 *  \a args as arguments.
 */
QString QDjangoUrlResolver::reverse(QObject *receiver, const char *member, const QVariantList &args) const
{
    QString path = d->reverse(receiver, member, args);
    if (path.isNull())
        return QString();
    else
        return QLatin1String("/") + path;
}


