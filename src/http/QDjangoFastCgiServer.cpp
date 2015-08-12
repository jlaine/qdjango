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

#include <QDebug>
#include <QLocalServer>
#include <QLocalSocket>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUrl>

#include "QDjangoFastCgiServer.h"
#include "QDjangoFastCgiServer_p.h"
#include "QDjangoHttpController.h"
#include "QDjangoHttpRequest.h"
#include "QDjangoHttpRequest_p.h"
#include "QDjangoHttpResponse.h"
#include "QDjangoHttpResponse_p.h"
#include "QDjangoHttpServer.h"
#include "QDjangoUrlResolver.h"

//#define QDJANGO_DEBUG_FCGI

quint16 QDjangoFastCgiHeader::contentLength(FCGI_Header *header)
{
    return (header->contentLengthB1 << 8) | header->contentLengthB0;
}

quint16 QDjangoFastCgiHeader::requestId(FCGI_Header *header)
{
    return (header->requestIdB1 << 8) | header->requestIdB0;
}

void QDjangoFastCgiHeader::setContentLength(FCGI_Header *header, quint16 contentLength)
{
    header->contentLengthB1 = (contentLength >> 8) & 0xff;
    header->contentLengthB0 = (contentLength & 0xff);
}

void QDjangoFastCgiHeader::setRequestId(FCGI_Header *header, quint16 requestId)
{
    header->requestIdB1 = (requestId >> 8) & 0xff;
    header->requestIdB0 = (requestId & 0xff);
}

#ifdef QDJANGO_DEBUG_FCGI
static void hDebug(FCGI_Header *header, const char *dir)
{
    qDebug("--- FCGI Record %s ---", dir);
    qDebug("version: %i", header->version);
    qDebug("type: %i", header->type);
    qDebug("requestId: %i", QDjangoFastCgiHeader::requestId(header));
    qDebug("contentLength: %i", QDjangoFastCgiHeader::contentLength(header));
    qDebug("paddingLength: %i", header->paddingLength);
}
#endif

/// \cond

QDjangoFastCgiConnection::QDjangoFastCgiConnection(QIODevice *device, QDjangoFastCgiServer *server)
    : QObject(server)
    , m_device(device)
    , m_inputPos(0)
    , m_keepConnection(false)
    , m_pendingRequest(0)
    , m_pendingRequestId(0)
    , m_server(server)
{
    bool check;
    Q_UNUSED(check);

    m_device->setParent(this);
    check = connect(m_device, SIGNAL(disconnected()),
                    this, SIGNAL(closed()));
    Q_ASSERT(check);

    check = connect(m_device, SIGNAL(bytesWritten(qint64)),
                    this, SLOT(_q_bytesWritten(qint64)));
    Q_ASSERT(check);

    check = connect(m_device, SIGNAL(readyRead()),
                    this, SLOT(_q_readyRead()));
    Q_ASSERT(check);
}

QDjangoFastCgiConnection::~QDjangoFastCgiConnection()
{
    if (m_pendingRequest)
        delete m_pendingRequest;
}

void QDjangoFastCgiConnection::writeResponse(quint16 requestId, QDjangoHttpResponse *response)
{
    // serialise HTTP response
    QString httpHeader = QString::fromLatin1("Status: %1 %2\r\n").arg(response->d->statusCode).arg(response->d->reasonPhrase);
    QList<QPair<QString, QString> >::ConstIterator it = response->d->headers.constBegin();
    while (it != response->d->headers.constEnd()) {
        httpHeader += (*it).first + QLatin1String(": ") + (*it).second + QLatin1String("\r\n");
        ++it;
    }
    const QByteArray data = httpHeader.toUtf8() + "\r\n" + response->d->body;

    const char *ptr = data.constData();
    FCGI_Header *header = (FCGI_Header*)m_outputBuffer;
    memset(header, 0, FCGI_HEADER_LEN);
    header->version = 1;
    QDjangoFastCgiHeader::setRequestId(header, requestId);

    for (qint64 bytesRemaining = data.size(); ; ) {
        const quint16 contentLength = qMin(bytesRemaining, qint64(32768));

        header->type = FCGI_STDOUT;
        QDjangoFastCgiHeader::setContentLength(header, contentLength);
        memcpy(m_outputBuffer + FCGI_HEADER_LEN, ptr, contentLength);
        m_device->write(m_outputBuffer, FCGI_HEADER_LEN + contentLength);
#ifdef QDJANGO_DEBUG_FCGI
        hDebug(header, "sent");
        qDebug("[STDOUT]");
#endif

        if (contentLength > 0) {
            ptr += contentLength;
            bytesRemaining -= contentLength;
        } else {
            break;
        }
    }

    quint16 contentLength = 8;
    header->type = FCGI_END_REQUEST;
    QDjangoFastCgiHeader::setContentLength(header, contentLength);
    memset(m_outputBuffer + FCGI_HEADER_LEN, 0, contentLength);
    m_device->write(m_outputBuffer, FCGI_HEADER_LEN + contentLength);
#ifdef QDJANGO_DEBUG_FCGI
    hDebug(header, "sent");
    qDebug("[END REQUEST]");
#endif
}

/** When bytes have been written, check whether we need to close
 *  the connection.
 *
 * @param bytes
 */
void QDjangoFastCgiConnection::_q_bytesWritten(qint64 bytes)
{
    Q_UNUSED(bytes);
    if (!m_device->bytesToWrite() && !m_keepConnection) {
#ifdef QDJANGO_DEBUG_FCGI
        qDebug("Closing connection");
#endif
        m_device->close();
        emit closed();
    }
}

void QDjangoFastCgiConnection::_q_readyRead()
{
    while (m_device->bytesAvailable()) {
        // read record header
        if (m_inputPos < FCGI_HEADER_LEN) {
            const qint64 length = m_device->read(m_inputBuffer + m_inputPos, FCGI_HEADER_LEN - m_inputPos);
            if (length < 0) {
                qWarning("Failed to read FastCGI record header from socket");
                m_device->close();
                emit closed();
                return;
            }
            m_inputPos += length;
            if (m_inputPos < FCGI_HEADER_LEN)
                return;
        }

        // read record body
        FCGI_Header *header = (FCGI_Header*)m_inputBuffer;
        const quint16 contentLength = QDjangoFastCgiHeader::contentLength(header);
        const quint16 bodyLength = contentLength + header->paddingLength;
        const qint64 length = m_device->read(m_inputBuffer + m_inputPos, bodyLength + FCGI_HEADER_LEN - m_inputPos);
        if (length < 0) {
            qWarning("Failed to read FastCGI record body from socket");
            m_device->close();
            emit closed();
            return;
        }
        m_inputPos += length;
        if (m_inputPos < FCGI_HEADER_LEN + bodyLength)
            return;
        m_inputPos = 0;

        // process record
#ifdef QDJANGO_DEBUG_FCGI
        hDebug(header, "received");
#endif
        if (header->version != 1) {
            qWarning("Received FastCGI record with an invalid version %i", header->version);
            m_device->close();
            emit closed();
            return;
        }
        const quint16 requestId = QDjangoFastCgiHeader::requestId(header);
        if (header->type != FCGI_BEGIN_REQUEST && (!m_pendingRequest || requestId != m_pendingRequestId)) {
            qWarning("Received FastCGI record for an invalid request %i", requestId);
            m_device->close();
            emit closed();
            return;
        }

        quint8 *d = (quint8*)(m_inputBuffer + FCGI_HEADER_LEN);
        switch (header->type) {
        case FCGI_BEGIN_REQUEST: {
            const quint8 flags = d[2];
#ifdef QDJANGO_DEBUG_FCGI
            const quint16 role = (d[0] << 8) | d[1];
            qDebug("[BEGIN REQUEST]");
            qDebug("role: %i", role);
            qDebug("flags: %i", flags);
#endif
            // we do not support multiplexing
            if (m_pendingRequest) {
                qWarning("Received new FastCGI request %i while already handling request %i", requestId, m_pendingRequestId);
                m_device->close();
                emit closed();
                return;
            }
            m_keepConnection = (flags & FCGI_KEEP_CONN);
            m_pendingRequest = new QDjangoHttpRequest;
            m_pendingRequestId = requestId;
            break;
        }
        case FCGI_ABORT_REQUEST:
#ifdef QDJANGO_DEBUG_FCGI
            qDebug("[ABORT]");
#endif
            m_device->close();
            emit closed();
            return;
        case FCGI_PARAMS:
#ifdef QDJANGO_DEBUG_FCGI
            qDebug("[PARAMS]");
#endif
            while (d < (quint8*)(m_inputBuffer + FCGI_HEADER_LEN + contentLength)) {
                quint32 nameLength;
                quint32 valueLength;
                if (d[0] >> 7) {
                    nameLength = ((d[0] & 0x7f) << 24) | (d[1] << 16) | (d[2] << 8) | d[3];
                    d += 4;
                } else {
                    nameLength = d[0];
                    d++;
                }
                if (d[0] >> 7) {
                    valueLength = ((d[0] & 0x7f) << 24) | (d[1] << 16) | (d[2] << 8) | d[3];
                    d += 4;
                } else {
                    valueLength = d[0];
                    d++;
                }
                const QByteArray name((char*)d, nameLength);
                d += nameLength;
                const QByteArray value((char*)d, valueLength);
                d += valueLength;

#ifdef QDJANGO_DEBUG_FCGI
                qDebug() << name << ":" << value;
#endif
                if (name == "PATH_INFO") {
                    m_pendingRequest->d->path = QString::fromUtf8(value);
                } else if (name == "REQUEST_URI") {
                    m_pendingRequest->d->path = QUrl(QString::fromUtf8(value)).path();
                } else if (name == "REQUEST_METHOD") {
                    m_pendingRequest->d->method = QString::fromUtf8(value);
                }
                m_pendingRequest->d->meta.insert(QString::fromLatin1(name), QString::fromUtf8(value));
            }
            break;
        case FCGI_STDIN:
#ifdef QDJANGO_DEBUG_FCGI
            qDebug("[STDIN]");
#endif
            if (contentLength) {
                m_pendingRequest->d->buffer.append((char*)d, contentLength);
            } else {
                // an empty STDIN record signals the end of the request
                QDjangoHttpRequest *request = m_pendingRequest;
                const quint16 requestId = m_pendingRequestId;

                m_pendingRequest = 0;
                m_pendingRequestId = 0;

                QDjangoHttpResponse *response = m_server->urls()->respond(*request, request->path());
                writeResponse(requestId, response);
            }
            break;
        default:
            qWarning("Received FastCGI record with an invalid type %i", header->type);
            m_device->close();
            emit closed();
            return;
        }
    }
}

/// \endcond

class QDjangoFastCgiServerPrivate
{
public:
    QDjangoFastCgiServerPrivate(QDjangoFastCgiServer *qq);
    QLocalServer *localServer;
    QTcpServer *tcpServer;
    QDjangoUrlResolver *urlResolver;

private:
    QDjangoFastCgiServer *q;
};

QDjangoFastCgiServerPrivate::QDjangoFastCgiServerPrivate(QDjangoFastCgiServer *qq)
    : localServer(0),
    tcpServer(0),
    q(qq)
{
    urlResolver = new QDjangoUrlResolver(q);
}

/** Constructs a new FastCGI server.
 */
QDjangoFastCgiServer::QDjangoFastCgiServer(QObject *parent)
    : QObject(parent)
{
    d = new QDjangoFastCgiServerPrivate(this);
}

/** Destroys the FastCGI server.
 */
QDjangoFastCgiServer::~QDjangoFastCgiServer()
{
    delete d;
}

/** Closes the server. The server will no longer listen for
 *  incoming connections.
 */
void QDjangoFastCgiServer::close()
{
    if (d->localServer)
        d->localServer->close();
    if (d->tcpServer)
        d->tcpServer->close();
}

/** Tells the server to listen for incoming connections on the given
 *  local socket.
 */
bool QDjangoFastCgiServer::listen(const QString &name)
{
    if (!d->localServer) {
        bool check;
        Q_UNUSED(check);

        d->localServer = new QLocalServer(this);
        check = connect(d->localServer, SIGNAL(newConnection()),
                        this, SLOT(_q_newLocalConnection()));
        Q_ASSERT(check);
    }

    return d->localServer->listen(name);
}

/** Tells the server to listen for incoming TCP connections on the given
 *  \a address and \a port.
 */
bool QDjangoFastCgiServer::listen(const QHostAddress &address, quint16 port)
{
    if (!d->tcpServer) {
        bool check;
        Q_UNUSED(check);

        d->tcpServer = new QTcpServer(this);
        check = connect(d->tcpServer, SIGNAL(newConnection()),
                        this, SLOT(_q_newTcpConnection()));
        Q_ASSERT(check);
    }

    return d->tcpServer->listen(address, port);
}

/** Returns the root URL resolver for the server, which dispatches
 *  requests to handlers.
 */
QDjangoUrlResolver* QDjangoFastCgiServer::urls() const
{
    return d->urlResolver;
}

void QDjangoFastCgiServer::_q_newLocalConnection()
{
    bool check;
    Q_UNUSED(check);

    QLocalSocket *socket;
    while ((socket = d->localServer->nextPendingConnection()) != 0) {
#ifdef QDJANGO_DEBUG_FCGI
        qDebug("New local connection");
#endif
        QDjangoFastCgiConnection *connection = new QDjangoFastCgiConnection(socket, this);
        check = connect(connection, SIGNAL(closed()),
                        connection, SLOT(deleteLater()));
        Q_ASSERT(check);
    }
}
void QDjangoFastCgiServer::_q_newTcpConnection()
{
    bool check;
    Q_UNUSED(check);

    QTcpSocket *socket;
    while ((socket = d->tcpServer->nextPendingConnection()) != 0) {
#ifdef QDJANGO_DEBUG_FCGI
        qDebug("New TCP connection");
#endif
        QDjangoFastCgiConnection *connection = new QDjangoFastCgiConnection(socket, this);
        check = connect(connection, SIGNAL(closed()),
                        connection, SLOT(deleteLater()));
        Q_ASSERT(check);
    }
}
