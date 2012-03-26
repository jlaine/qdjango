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

#ifndef QDJANGO_URL_RESOLVER_H
#define QDJANGO_URL_RESOLVER_H

#include <QObject>
#include <QVariant>

class QDjangoHttpRequest;
class QDjangoHttpResponse;
class QDjangoUrlResolverPrivate;
class QRegExp;

/** \brief The QDjangoUrlResolver class maps incoming HTTP requests to handlers.
 *
 * \ingroup Http
 */
class QDjangoUrlResolver : public QObject
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
