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

#ifndef QDJANGO_URL_RESOLVER_H
#define QDJANGO_URL_RESOLVER_H

#include <QObject>
#include <QVariant>

#include "QDjangoHttp_p.h"

class QDjangoHttpRequest;
class QDjangoHttpResponse;
class QDjangoUrlResolverPrivate;
class QRegExp;

/** \brief The QDjangoUrlResolver class maps incoming HTTP requests to handlers.
 *
 * \ingroup Http
 */
class QDJANGO_HTTP_EXPORT QDjangoUrlResolver : public QObject
{
    Q_OBJECT

public:
    QDjangoUrlResolver(QObject *parent = 0);
    ~QDjangoUrlResolver();

    bool include(const QRegExp &path, QDjangoUrlResolver *urls);
    bool set(const QRegExp &path, QObject *receiver, const char *member);
    QString reverse(QObject *receiver, const char *member, const QVariantList &args = QVariantList()) const;

public slots:
    QDjangoHttpResponse* respond(const QDjangoHttpRequest &request, const QString &path) const;

private:
    QDjangoUrlResolverPrivate *d;
    friend class QDjangoUrlResolverPrivate;
};


#endif
