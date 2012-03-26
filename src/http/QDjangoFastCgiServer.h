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

#ifndef QDJANGO_FASTCGI_SERVER_H
#define QDJANGO_FASTCGI_SERVER_H

#include <QHostAddress>
#include <QObject>

class QDjangoFastCgiServerPrivate;
class QDjangoHttpController;
class QDjangoUrlResolver;

/** \brief The QDjangoFastCgiServer class represents a FastCGI server.
 *
 * \ingroup Http
 */
class QDjangoFastCgiServer : public QObject
{
    Q_OBJECT

public:
    QDjangoFastCgiServer(QObject *parent = 0);
    ~QDjangoFastCgiServer();

    void close();
    bool listen(const QString &name);
    bool listen(const QHostAddress &address, quint16 port);
    QDjangoUrlResolver *urls() const;

private slots:
    void _q_newLocalConnection();
    void _q_newTcpConnection();

private:
    Q_DISABLE_COPY(QDjangoFastCgiServer)
    QDjangoFastCgiServerPrivate *d;
};

#endif
