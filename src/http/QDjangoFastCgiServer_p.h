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

#ifndef QDJANGO_FASTCGI_SERVER_P_H
#define QDJANGO_FASTCGI_SERVER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QDjango API.
//

#include "QDjangoHttp_p.h"
#include <QObject>

#define FCGI_HEADER_LEN  8
#define FCGI_RECORD_SIZE (255*255 + 255 + 8)

#define FCGI_BEGIN_REQUEST       1
#define FCGI_ABORT_REQUEST       2
#define FCGI_END_REQUEST         3
#define FCGI_PARAMS              4
#define FCGI_STDIN               5
#define FCGI_STDOUT              6

#define FCGI_KEEP_CONN 1

class QDjangoFastCgiServer;
class QDjangoHttpRequest;
class QDjangoHttpResponse;
class QIODevice;

typedef struct {
    unsigned char version;
    unsigned char type;
    unsigned char requestIdB1;
    unsigned char requestIdB0;
    unsigned char contentLengthB1;
    unsigned char contentLengthB0;
    unsigned char paddingLength;
    unsigned char reserved;
} FCGI_Header;

class QDJANGO_HTTP_AUTOTEST_EXPORT QDjangoFastCgiHeader
{
public:
    static quint16 contentLength(FCGI_Header *header);
    static quint16 requestId(FCGI_Header *header);
    static void setContentLength(FCGI_Header *header, quint16 contentLength);
    static void setRequestId(FCGI_Header *header, quint16 requestId);
};

class QDJANGO_HTTP_AUTOTEST_EXPORT QDjangoFastCgiConnection : public QObject
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
    bool m_keepConnection;
    char m_outputBuffer[FCGI_RECORD_SIZE];
    QDjangoHttpRequest *m_pendingRequest;
    quint16 m_pendingRequestId;
    QDjangoFastCgiServer *m_server;
};

#endif
