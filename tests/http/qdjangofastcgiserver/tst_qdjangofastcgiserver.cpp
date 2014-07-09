/*
 * Copyright (C) 2010-2014 Jeremy Lainé
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

#include <QLocalSocket>
#include <QTcpSocket>
#include <QtTest>
#include <QUrl>

#include "QDjangoFastCgiServer.h"
#include "QDjangoFastCgiServer_p.h"
#include "QDjangoHttpController.h"
#include "QDjangoHttpRequest.h"
#include "QDjangoHttpResponse.h"
#include "QDjangoFastCgiServer.h"
#include "QDjangoUrlResolver.h"

#define ERROR_DATA QByteArray("Status: 500 Internal Server Error\r\n" \
    "Content-Length: 107\r\n" \
    "Content-Type: text/html; charset=utf-8\r\n" \
    "\r\n" \
    "<html><head><title>Error</title></head><body><p>An internal server error was encountered.</p></body></html>")

#define NOT_FOUND_DATA QByteArray("Status: 404 Not Found\r\n" \
    "Content-Length: 107\r\n" \
    "Content-Type: text/html; charset=utf-8\r\n" \
    "\r\n" \
    "<html><head><title>Error</title></head><body><p>The document you requested was not found.</p></body></html>")

#define ROOT_DATA QByteArray("Status: 200 OK\r\n" \
    "Content-Length: 17\r\n" \
    "Content-Type: text/plain\r\n" \
    "\r\n" \
    "method=GET|path=/")

#define QUERY_STRING_DATA QByteArray("Status: 200 OK\r\n" \
    "Content-Length: 25\r\n" \
    "Content-Type: text/plain\r\n" \
    "\r\n" \
    "method=GET|path=/|get=bar")

class QDjangoFastCgiReply : public QObject
{
    Q_OBJECT

public:
    QDjangoFastCgiReply(QObject *parent = 0)
        : QObject(parent) {};

    QByteArray data;

signals:
    void finished();
};

class QDjangoFastCgiClient : public QObject
{
    Q_OBJECT

public:
    QDjangoFastCgiClient(QIODevice *socket);
    QDjangoFastCgiReply* get(const QUrl &url);
    QDjangoFastCgiReply* post(const QUrl &url, const QByteArray &data);

private slots:
    void _q_readyRead();

private:
    QDjangoFastCgiReply* request(const QByteArray &method, const QUrl &url, const QByteArray &data);

    QIODevice *m_device;
    QMap<quint16, QDjangoFastCgiReply*> m_replies;
    quint16 m_requestId;
};

QDjangoFastCgiClient::QDjangoFastCgiClient(QIODevice *socket)
    : m_device(socket)
    , m_requestId(0)
{
    connect(socket, SIGNAL(readyRead()), this, SLOT(_q_readyRead()));
};

QDjangoFastCgiReply* QDjangoFastCgiClient::get(const QUrl &url)
{
    return request("GET", url, QByteArray());
}

QDjangoFastCgiReply* QDjangoFastCgiClient::post(const QUrl &url, const QByteArray &data)
{
    return request("POST", url, data);
}

QDjangoFastCgiReply* QDjangoFastCgiClient::request(const QByteArray &method, const QUrl &url, const QByteArray &data)
{
    const quint16 requestId = ++m_requestId;

    QDjangoFastCgiReply *reply = new QDjangoFastCgiReply(this);
    m_replies[requestId] = reply;
    QByteArray headerBuffer(FCGI_HEADER_LEN, '\0');
    FCGI_Header *header = (FCGI_Header*)headerBuffer.data();

    QByteArray ba;

    // BEGIN REQUEST
    ba = QByteArray("\x01\x00\x00\x00\x00\x00\x00\x00", 8);
    header->version = 1;
    FCGI_Header_setRequestId(header, requestId);
    header->type = FCGI_BEGIN_REQUEST;
    FCGI_Header_setContentLength(header, ba.size());
    m_device->write(headerBuffer + ba);

    QMap<QByteArray, QByteArray> params;
    params["PATH_INFO"] = url.path().toUtf8();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    params["QUERY_STRING"] = url.query().toUtf8();
#else
    params["QUERY_STRING"] = url.encodedQuery();
#endif
    params["REQUEST_URI"] = url.toString().toUtf8();
    params["REQUEST_METHOD"] = method;

    ba.clear();
    foreach (const QByteArray &key, params.keys()) {
        const QByteArray value = params.value(key);
        ba.append(char(key.size()));
        ba.append(char(value.size()));
        ba.append(key);
        ba.append(value);
    }

    // FAST CGI PARAMS
    header->type = FCGI_PARAMS;
    FCGI_Header_setContentLength(header, ba.size());
    m_device->write(headerBuffer + ba);

    // STDIN
    header->type = FCGI_STDIN;
    FCGI_Header_setContentLength(header, data.size());
    m_device->write(headerBuffer + data);

    return reply;
}


void QDjangoFastCgiClient::_q_readyRead()
{
    char inputBuffer[FCGI_RECORD_SIZE];
    FCGI_Header *header = (FCGI_Header*)inputBuffer;

    while (m_device->bytesAvailable()) {
        if (m_device->read(inputBuffer, FCGI_HEADER_LEN) != FCGI_HEADER_LEN) {
            qWarning("header read fail");
            return;
        }

        const quint16 contentLength = FCGI_Header_contentLength(header);
        const quint16 requestId = FCGI_Header_requestId(header);
        const quint16 bodyLength = contentLength + header->paddingLength;
        if (m_device->read(inputBuffer + FCGI_HEADER_LEN, bodyLength) != bodyLength) {
            qWarning("body read fail");
            return;
        }
        if (!m_replies.contains(requestId)) {
            qWarning() << "unknown request" << requestId;
            return;
        }
        if (header->type == FCGI_STDOUT) {
            const QByteArray data = QByteArray(inputBuffer + FCGI_HEADER_LEN, contentLength);
            m_replies[requestId]->data += data;
        } else if (header->type == FCGI_END_REQUEST) {
            QMetaObject::invokeMethod(m_replies[requestId], "finished");
        }
    }
}

/** Test QDjangoFastCgiServer class.
 */
class tst_QDjangoFastCgiServer : public QObject
{
    Q_OBJECT

private slots:
    void cleanup();
    void init();
    void testLocal_data();
    void testLocal();
    void testPost();
    void testTcp_data();
    void testTcp();

    QDjangoHttpResponse* _q_index(const QDjangoHttpRequest &request);
    QDjangoHttpResponse* _q_error(const QDjangoHttpRequest &request);

private:
    QDjangoFastCgiServer *server;
};

void tst_QDjangoFastCgiServer::cleanup()
{
    server->close();
    delete server;
}

void tst_QDjangoFastCgiServer::init()
{
    server = new QDjangoFastCgiServer;
    server->urls()->set(QRegExp(QLatin1String(QLatin1String("^$"))), this, "_q_index");
    server->urls()->set(QRegExp(QLatin1String("^internal-server-error$")), this, "_q_error");
}

void tst_QDjangoFastCgiServer::testLocal_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<QByteArray>("data");
    QTest::newRow("root") << "/" << ROOT_DATA;
    QTest::newRow("query-string") << "/?message=bar" << QUERY_STRING_DATA;
    QTest::newRow("not-found") << "/not-found" << NOT_FOUND_DATA;
    QTest::newRow("internal-server-error") << "/internal-server-error" << ERROR_DATA;
}

void tst_QDjangoFastCgiServer::testLocal()
{
    QFETCH(QString, path);
    QFETCH(QByteArray, data);

    const QString name("/tmp/qdjangofastcgi.socket");
    QCOMPARE(server->listen(name), true);
    
    QLocalSocket socket;
    socket.connectToServer(name);
    QCOMPARE(socket.state(), QLocalSocket::ConnectedState);

    QDjangoFastCgiClient client(&socket);
    QDjangoFastCgiReply *reply = client.get(path);

    QEventLoop loop;
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    QCOMPARE(reply->data, data);
}

void tst_QDjangoFastCgiServer::testPost()
{
    const QString name("/tmp/qdjangofastcgi.socket");

    QCOMPARE(server->listen(name), true);

    QLocalSocket socket;
    socket.connectToServer(name);
    QCOMPARE(socket.state(), QLocalSocket::ConnectedState);

    QDjangoFastCgiClient client(&socket);
    QDjangoFastCgiReply *reply = client.post(QUrl("/"), QByteArray());

    QEventLoop loop;
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    QCOMPARE(reply->data, QByteArray("Status: 200 OK\r\n" \
        "Content-Length: 18\r\n" \
        "Content-Type: text/plain\r\n" \
        "\r\n" \
        "method=POST|path=/"));
}

void tst_QDjangoFastCgiServer::testTcp_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<QByteArray>("data");
    QTest::newRow("root") << "/" << ROOT_DATA;
    QTest::newRow("query-string") << "/?message=bar" << QUERY_STRING_DATA;
    QTest::newRow("not-found") << "/not-found" << NOT_FOUND_DATA;
    QTest::newRow("internal-server-error") << "/internal-server-error" << ERROR_DATA;
}

void tst_QDjangoFastCgiServer::testTcp()
{
    QFETCH(QString, path);
    QFETCH(QByteArray, data);

    QCOMPARE(server->listen(QHostAddress::LocalHost, 8123), true);

    QTcpSocket socket;
    socket.connectToHost("127.0.0.1", 8123);

    QDjangoFastCgiClient client(&socket);

    QEventLoop loop;
    QObject::connect(&socket, SIGNAL(connected()), &loop, SLOT(quit()));
    loop.exec();

    QCOMPARE(socket.state(), QAbstractSocket::ConnectedState);

    QDjangoFastCgiReply *reply = client.get(path);
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    QCOMPARE(reply->data, data);
}

QDjangoHttpResponse *tst_QDjangoFastCgiServer::_q_index(const QDjangoHttpRequest &request)
{
    QDjangoHttpResponse *response = new QDjangoHttpResponse;
    response->setHeader(QLatin1String("Content-Type"), QLatin1String("text/plain"));

    QString output = QLatin1String("method=") + request.method();
    output += QLatin1String("|path=") + request.path();

    const QString getValue = request.get(QLatin1String("message"));
    if (!getValue.isEmpty())
        output += QLatin1String("|get=") + getValue;

    const QString postValue = request.post(QLatin1String("message"));
    if (!postValue.isEmpty())
        output += QLatin1String("|post=") + postValue;

    response->setBody(output.toUtf8());
    return response;
}

QDjangoHttpResponse *tst_QDjangoFastCgiServer::_q_error(const QDjangoHttpRequest &request)
{
    Q_UNUSED(request);

    return QDjangoHttpController::serveInternalServerError(request);
}


QTEST_MAIN(tst_QDjangoFastCgiServer)
#include "tst_qdjangofastcgiserver.moc"
