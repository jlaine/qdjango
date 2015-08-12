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
#include <QStringList>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUrl>

#include "QDjangoHttpController.h"
#include "QDjangoHttpRequest.h"
#include "QDjangoHttpRequest_p.h"
#include "QDjangoHttpResponse.h"
#include "QDjangoHttpResponse_p.h"
#include "QDjangoHttpServer.h"
#include "QDjangoHttpServer_p.h"
#include "QDjangoUrlResolver.h"

//#define QDJANGO_DEBUG_HTTP

// maximum request body size is 10 MB
#define MAX_BODY_SIZE (10 * 1024 * 1024)

/// \cond

/** Constructs a new HTTP connection.
 */
QDjangoHttpConnection::QDjangoHttpConnection(QTcpSocket *device, QDjangoHttpServer *server)
    : QObject(server),
    m_closeAfterResponse(false),
    m_pendingRequest(0),
    m_requestCount(0),
    m_server(server),
    m_socket(device)
{
    bool check;
    Q_UNUSED(check);

    m_socket->setParent(this);
    check = connect(m_socket, SIGNAL(bytesWritten(qint64)),
                    this, SLOT(_q_bytesWritten(qint64)));
    Q_ASSERT(check);

    check = connect(m_socket, SIGNAL(disconnected()),
                    this, SIGNAL(closed()));
    Q_ASSERT(check);

    check = connect(m_socket, SIGNAL(readyRead()),
                    this, SLOT(_q_readyRead()));
    Q_ASSERT(check);
}

/** Destroys the HTTP connection.
 */
QDjangoHttpConnection::~QDjangoHttpConnection()
{
    if (m_pendingRequest)
        delete m_pendingRequest;
    foreach (const QDjangoHttpJob &job, m_pendingJobs) {
        delete job.first;
        delete job.second;
    }
}

/** When bytes have been written, check whether we need to close
 *  the connection.
 *
 * @param bytes
 */
void QDjangoHttpConnection::_q_bytesWritten(qint64 bytes)
{
    Q_UNUSED(bytes);
    if (!m_socket->bytesToWrite()) {
        if (!m_pendingJobs.isEmpty()) {
            _q_writeResponse();
        } else if (m_closeAfterResponse) {
#ifdef QDJANGO_DEBUG_HTTP
            qDebug("Closing connection");
#endif
            m_socket->close();
            emit closed();
        }
    }
}

/** Handle incoming data on the socket.
 */
void QDjangoHttpConnection::_q_readyRead()
{
    QDjangoHttpRequest *request = m_pendingRequest;
    if (!request) {
        request = new QDjangoHttpRequest;
        m_requestBytesRemaining = 0;
        m_requestHeaderLine = 0;
        m_requestHeaderReceived = false;
        m_requestHeaders.clear();
        m_requestMajorVersion = 0;
        m_requestMinorVersion = 0;
        m_requestPath.clear();
    }

    // Read request header
    while (!m_requestHeaderReceived && m_socket->canReadLine()) {
        const QString line = QString::fromUtf8(m_socket->readLine());

        if (!m_requestHeaderLine++) {
            bool ok = false;
            QStringList lst = line.simplified().split(QLatin1String(" "));
            if (lst.count() > 0) {
                request->d->method = lst[0];
                if (lst.count() > 1) {
                    m_requestPath = lst[1];
                    request->d->path = QUrl(m_requestPath).path();
                    if (lst.count() > 2) {
                        QString v = lst[2];
                        if (v.length() >= 8 && v.left(5) == QLatin1String("HTTP/") &&
                            v[5].isDigit() && v[6] == QLatin1Char('.') && v[7].isDigit()) {
                            m_requestMajorVersion = v[5].toLatin1() - '0';
                            m_requestMinorVersion = v[7].toLatin1() - '0';
                            ok = true;
                        }
                    }
                }
            }
            if (!ok) {
                qWarning("Invalid HTTP request");
                m_socket->close();
                return;
            }
        } else if (line != QLatin1String("\r\n")) {
            int i = line.indexOf(QLatin1Char(':'));
            if (i == -1) {
                qWarning("Invalid HTTP request header");
                m_socket->close();
                return;
            }
            const QString key = line.left(i).trimmed();
            const QString value = line.mid(i + 1).trimmed();
            m_requestHeaders.append(qMakePair(key, value));

            if (key.toLower() == QLatin1String("content-length")) {
                m_requestBytesRemaining = value.toInt();
            }
        } else {
            if (m_requestBytesRemaining < 0 || m_requestBytesRemaining > MAX_BODY_SIZE) {
                qWarning("Invalid Content-Length");
                m_socket->close();
                return;
            }
            m_requestHeaderReceived = true;
        }
    }
    if (!m_requestHeaderReceived) {
        m_pendingRequest = request;
        return;
    }

    // Read request body
    if (m_requestBytesRemaining > 0) {
        const QByteArray chunk = m_socket->read(m_requestBytesRemaining);
        request->d->buffer += chunk;
        m_requestBytesRemaining -= chunk.size();
    }
    if (m_requestBytesRemaining) {
        m_pendingRequest = request;
        return;
    }
    m_pendingRequest = 0;

#ifdef QDJANGO_DEBUG_HTTP
    qDebug("Handling request %i", d->requestCount++);
#endif

    /* Map meta-information */
    QString metaKey;
    QList<QPair<QString, QString> >::ConstIterator it = m_requestHeaders.constBegin();
    while (it != m_requestHeaders.constEnd()) {
        if (it->first == QLatin1String("Content-Length"))
            metaKey = QLatin1String("CONTENT_LENGTH");
        else if (it->first == QLatin1String("Content-Type"))
            metaKey = QLatin1String("CONTENT_TYPE");
        else {
            metaKey = QLatin1String("HTTP_") + it->first.toUpper();
            metaKey.replace(QLatin1Char('-'), QLatin1Char('_'));
        }
        request->d->meta.insert(metaKey, it->second);
        ++it;
    }

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    request->d->meta.insert(QLatin1String("QUERY_STRING"), QUrl(m_requestPath).query());
#else
    request->d->meta.insert(QLatin1String("QUERY_STRING"), QString::fromLatin1(QUrl(m_requestPath).encodedQuery()));
#endif
    request->d->meta.insert(QLatin1String("REMOTE_ADDR"), m_socket->peerAddress().toString());
    request->d->meta.insert(QLatin1String("REQUEST_METHOD"), request->method());
    request->d->meta.insert(QLatin1String("SERVER_NAME"), m_socket->localAddress().toString());
    request->d->meta.insert(QLatin1String("SERVER_PORT"), QString::number(m_socket->localPort()));

    /* Process request */
    bool keepAlive = m_requestMajorVersion >= 1 && m_requestMinorVersion >= 1;
    if (request->d->meta.value(QLatin1String("HTTP_CONNECTION")).toLower() == QLatin1String("keep-alive"))
        keepAlive = true;
    else if (request->d->meta.value(QLatin1String("HTTP_CONNECTION")).toLower() == QLatin1String("close"))
        keepAlive = false;

    QDjangoHttpResponse *response = m_server->urls()->respond(*request, request->path());
    m_pendingJobs << qMakePair(request, response);

    /* Store keep-alive flag */
    if (!keepAlive)
        m_closeAfterResponse = true;

    connect(response, SIGNAL(ready()), this, SLOT(_q_writeResponse()));
    _q_writeResponse();
}

void QDjangoHttpConnection::_q_writeResponse()
{
    while (!m_pendingJobs.isEmpty() &&
            m_pendingJobs.first().second->isReady()) {
        const QDjangoHttpJob job = m_pendingJobs.takeFirst();
        QDjangoHttpRequest *request = job.first;
        QDjangoHttpResponse *response = job.second;
        if (!response->isReady())
            return;

        /* Finalise response */
        response->setHeader(QLatin1String("Date"), QDjangoHttpController::httpDateTime(QDateTime::currentDateTime()));
        response->setHeader(QLatin1String("Server"), QString::fromLatin1("%1/%2").arg(qApp->applicationName(), qApp->applicationVersion()));
        response->setHeader(QLatin1String("Connection"), QLatin1String(m_closeAfterResponse ? "close" : "keep-alive"));

        /* Send response */
        QString httpHeader = QString::fromLatin1("HTTP/1.1 %1 %2\r\n").arg(response->d->statusCode).arg(response->d->reasonPhrase);
        QList<QPair<QString, QString> >::ConstIterator it = response->d->headers.constBegin();
        while (it != response->d->headers.constEnd()) {
            httpHeader += (*it).first + QLatin1String(": ") + (*it).second + QLatin1String("\r\n");
            ++it;
        }
        m_socket->write(httpHeader.toUtf8() + "\r\n" + response->d->body);

        /* Emit signal */
        emit requestFinished(request, response);

        /* Destroy response */
        delete request;
        response->deleteLater();
    }
}

/// \endcond

class QDjangoHttpServerPrivate
{
public:
    int connectionCount;
    QTcpServer *tcpServer;
    QDjangoUrlResolver *urlResolver;
};

/** Constructs a new HTTP server.
 */
QDjangoHttpServer::QDjangoHttpServer(QObject *parent)
    : QObject(parent),
    d(new QDjangoHttpServerPrivate)
{
    d->connectionCount = 0;
    d->tcpServer = 0;
    d->urlResolver = new QDjangoUrlResolver(this);
}

/** Destroys the HTTP server.
 */
QDjangoHttpServer::~QDjangoHttpServer()
{
    delete d;
}

/** Closes the server. The server will no longer listen for
 *  incoming connections.
 */
void QDjangoHttpServer::close()
{
    if (d->tcpServer)
        d->tcpServer->close();
}

/** Tells the server to listen for incoming TCP connections on the given
 *  \a address and \a port.
 */
bool QDjangoHttpServer::listen(const QHostAddress &address, quint16 port)
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

/** Returns the server's address if the server is listening for connections;
 *  otherwise returns QHostAddress::Null.
 */
QHostAddress QDjangoHttpServer::serverAddress() const
{
    if (!d->tcpServer)
        return QHostAddress::Null;
    return d->tcpServer->serverAddress();
}

/** Returns the server's port if the server is listening for connections;
 *  otherwise returns 0.
 */
quint16 QDjangoHttpServer::serverPort() const
{
    if (!d->tcpServer)
        return 0;
    return d->tcpServer->serverPort();
}

/** Returns the root URL resolver for the server, which dispatches
 *  requests to handlers.
 */
QDjangoUrlResolver* QDjangoHttpServer::urls() const
{
    return d->urlResolver;
}

/** Handles the creation of new HTTP connections.
 */
void QDjangoHttpServer::_q_newTcpConnection()
{
    bool check;
    Q_UNUSED(check);

    QTcpSocket *socket;
    while ((socket = d->tcpServer->nextPendingConnection()) != 0) {
        QDjangoHttpConnection *connection = new QDjangoHttpConnection(socket, this);
#ifdef QDJANGO_DEBUG_HTTP
        qDebug("Handling connection %i", d->connectionCount++);
#endif

        check = connect(connection, SIGNAL(closed()),
                        connection, SLOT(deleteLater()));
        Q_ASSERT(check);

        check = connect(connection, SIGNAL(requestFinished(QDjangoHttpRequest*,QDjangoHttpResponse*)),
                        this, SIGNAL(requestFinished(QDjangoHttpRequest*,QDjangoHttpResponse*)));
        Q_ASSERT(check);
    }
}
