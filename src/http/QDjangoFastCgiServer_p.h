/*
 * Copyright (C) 2010-2012 Jeremy Lainé
 * Contact: http://code.google.com/p/qdjango/
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

#ifndef QDJANGO_FASTCGI_SERVER_P_H
#define QDJANGO_FASTCGI_SERVER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QDjango API.
//

#include <QObject>

#define FCGI_RECORD_SIZE (255*255 + 255 + 8)

class QDjangoFastCgiServer;
class QDjangoHttpRequest;
class QDjangoHttpResponse;
class QIODevice;

class QDjangoFastCgiConnection : public QObject
{
    Q_OBJECT

public:
    QDjangoFastCgiConnection(QIODevice *device, QDjangoFastCgiServer *server);
    ~QDjangoFastCgiConnection();

signals:
    void closed();

private slots:
    void _q_bytesWritten(qint64 bytes);
    void _q_readyRead();

private:
    void writeResponse(quint16 requestId, QDjangoHttpResponse *response);

    QIODevice *m_device;
    char m_inputBuffer[FCGI_RECORD_SIZE];
    int m_inputPos;
    char m_outputBuffer[FCGI_RECORD_SIZE];
    QDjangoHttpRequest *m_pendingRequest;
    quint16 m_pendingRequestId;
    QDjangoFastCgiServer *m_server;
};

#endif
