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
#include "QDjangoHttpController.h"
#include "QDjangoHttpRequest.h"
#include "QDjangoHttpResponse.h"
#include "QDjangoFastCgiServer.h"
#include "QDjangoUrlResolver.h"

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

class QDjangoFastCgiClient : public QObject
{
    Q_OBJECT

public:
    QDjangoFastCgiClient(QIODevice *socket);
    void get(const QString &path);

private:
    QIODevice *m_device;
};

QDjangoFastCgiClient::QDjangoFastCgiClient(QIODevice *socket)
    : m_device(socket)
{
};

void QDjangoFastCgiClient::get(const QString &path)
{
    QByteArray headerBuffer(8, '\0');
    FCGI_Header *header = (FCGI_Header*)headerBuffer.data();

    QByteArray ba;

    // BEGIN REQUEST
    ba = QByteArray("\x01\x00\x00\x00\x00\x00\x00\x00", 8);
    header->version = 1;
    header->requestIdB0 = 1;
    header->requestIdB1 = 0;
    header->type = 0x01;
    header->contentLengthB0 = ba.size();
    header->contentLengthB1 = 0;
    m_device->write(headerBuffer + ba);

    QMap<QByteArray, QByteArray> params;
    params["PATH_INFO"] = path.toUtf8();
    params["REQUEST_METHOD"] = "GET";

    ba.clear();
    foreach (const QByteArray &key, params.keys()) {
        const QByteArray value = params.value(key);
        ba.append(char(key.size()));
        ba.append(char(value.size()));
        ba.append(key);
        ba.append(value);
    }

    // FAST CGI PARAMS
    header->type = 0x04;
    header->contentLengthB0 = ba.size();
    m_device->write(headerBuffer + ba);

    // STDIN
    header->type = 0x05;
    header->contentLengthB0 = 0;
    m_device->write(headerBuffer);
}

/** Test QDjangoFastCgiServer class.
 */
class tst_QDjangoFastCgiServer : public QObject
{
    Q_OBJECT

private slots:
    void cleanupTestCase();
    void initTestCase();
    void testLocal();
    void testTcp();

    QDjangoHttpResponse* _q_index(const QDjangoHttpRequest &request);
    QDjangoHttpResponse* _q_error(const QDjangoHttpRequest &request);

private:
    QDjangoFastCgiServer *server;
};


void tst_QDjangoFastCgiServer::cleanupTestCase()
{
    server->close();
    delete server;
}

void tst_QDjangoFastCgiServer::initTestCase()
{
    server = new QDjangoFastCgiServer;
    server->urls()->set(QRegExp(QLatin1String(QLatin1String("^$"))), this, "_q_index");
    server->urls()->set(QRegExp(QLatin1String("^internal-server-error$")), this, "_q_error");
}

void tst_QDjangoFastCgiServer::testLocal()
{
    const QString name("/tmp/qdjangofastcgi.socket");
    QCOMPARE(server->listen(name), true);
    
    QLocalSocket socket;
    socket.connectToServer(name);

    QDjangoFastCgiClient client(&socket);

    QCOMPARE(socket.state(), QLocalSocket::ConnectedState);
    client.get("/");

    QEventLoop loop;
    QObject::connect(&socket, SIGNAL(readyRead()), &loop, SLOT(quit()));
    loop.exec();
}

void tst_QDjangoFastCgiServer::testTcp()
{
    QCOMPARE(server->listen(QHostAddress::LocalHost, 8123), true);

    QTcpSocket socket;
    socket.connectToHost("127.0.0.1", 8123);

    QDjangoFastCgiClient client(&socket);

    QEventLoop loop;
    QObject::connect(&socket, SIGNAL(connected()), &loop, SLOT(quit()));
    loop.exec();

    QCOMPARE(socket.state(), QAbstractSocket::ConnectedState);
    client.get("/");
    
    QObject::connect(&socket, SIGNAL(readyRead()), &loop, SLOT(quit()));
    loop.exec();
}

QDjangoHttpResponse *tst_QDjangoFastCgiServer::_q_index(const QDjangoHttpRequest &request)
{
    QDjangoHttpResponse *response = new QDjangoHttpResponse;
    response->setHeader(QLatin1String("Content-Type"), QLatin1String("text/plain"));

    qDebug() << "INDEX";
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
