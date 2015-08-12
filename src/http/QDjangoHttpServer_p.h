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

#ifndef QDJANGO_HTTP_SERVER_P_H
#define QDJANGO_HTTP_SERVER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QDjango API.
//

#include <QObject>
#include <QList>
#include <QPair>
#include <QString>

class QDjangoHttpRequest;
class QDjangoHttpResponse;
class QDjangoHttpServer;
class QTcpSocket;

typedef QPair<QDjangoHttpRequest*,QDjangoHttpResponse*> QDjangoHttpJob;

/** \internal
 */
class QDjangoHttpConnection : public QObject
{
    Q_OBJECT

public:
    QDjangoHttpConnection(QTcpSocket *device, QDjangoHttpServer *server);
    ~QDjangoHttpConnection();

signals:
    /** This signal is emitted when the connection is closed.
     */
    void closed();

    /** This signal is emitted when a request completes.
     */
    void requestFinished(QDjangoHttpRequest *request, QDjangoHttpResponse *response);

private slots:
    void _q_bytesWritten(qint64 bytes);
    void _q_readyRead();
    void _q_writeResponse();

private:
    Q_DISABLE_COPY(QDjangoHttpConnection)
    bool m_closeAfterResponse;
    QList<QDjangoHttpJob> m_pendingJobs;
    QDjangoHttpRequest *m_pendingRequest;
    int m_requestCount;
    QDjangoHttpServer *m_server;
    QTcpSocket *m_socket;

    // request parsing
    qint64 m_requestBytesRemaining;
    int m_requestHeaderLine;
    bool m_requestHeaderReceived;
    QList<QPair<QString, QString> > m_requestHeaders;
    int m_requestMajorVersion;
    int m_requestMinorVersion;
    QString m_requestPath;
};

#endif
