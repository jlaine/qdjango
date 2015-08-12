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

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QtTest>
#include <QUrl>

#include "QDjangoHttpController.h"
#include "QDjangoHttpRequest.h"
#include "QDjangoHttpResponse.h"
#include "QDjangoHttpServer.h"
#include "QDjangoUrlResolver.h"

/** Test QDjangoHttpServer class.
 */
class tst_QDjangoHttpServer : public QObject
{
    Q_OBJECT

private slots:
    void cleanupTestCase();
    void initTestCase();
    void testCloseConnection();
    void testGet_data();
    void testGet();
    void testPost_data();
    void testPost();

    QDjangoHttpResponse* _q_index(const QDjangoHttpRequest &request);
    QDjangoHttpResponse* _q_error(const QDjangoHttpRequest &request);

private:
    QDjangoHttpServer *httpServer;
};


void tst_QDjangoHttpServer::cleanupTestCase()
{
    httpServer->close();
    delete httpServer;
}

void tst_QDjangoHttpServer::initTestCase()
{
    httpServer = new QDjangoHttpServer;
    httpServer->urls()->set(QRegExp(QLatin1String("^$")), this, "_q_index");
    httpServer->urls()->set(QRegExp(QLatin1String("^internal-server-error$")), this, "_q_error");
    QCOMPARE(httpServer->serverAddress(), QHostAddress(QHostAddress::Null));
    QCOMPARE(httpServer->serverPort(), quint16(0));
    QCOMPARE(httpServer->listen(QHostAddress::LocalHost, 8123), true);
    QCOMPARE(httpServer->serverAddress(), QHostAddress(QHostAddress::LocalHost));
    QCOMPARE(httpServer->serverPort(), quint16(8123));
}

void tst_QDjangoHttpServer::testCloseConnection()
{
    QNetworkAccessManager network;

    QNetworkRequest req(QUrl("http://127.0.0.1:8123/"));
    req.setRawHeader("Connection", "close");

    QNetworkReply *reply = network.get(req);

    QEventLoop loop;
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    QVERIFY(reply);
    QCOMPARE(reply->error(), QNetworkReply::NoError);
    //QCOMPARE(reply->readAll(), body);
    delete reply;

}

void tst_QDjangoHttpServer::testGet_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<int>("err");
    QTest::addColumn<QByteArray>("body");

    const QString errorTemplate = QLatin1String(
        "<html>"
        "<head><title>Error</title></head>"
        "<body><p>%1</p></body>"
        "</html>");
#if (QT_VERSION >= QT_VERSION_CHECK(5, 3, 0))
    int internalServerError = int(QNetworkReply::InternalServerError);
#else
    int internalServerError = int(QNetworkReply::UnknownContentError);
#endif
    QTest::newRow("root") << "/" << int(QNetworkReply::NoError) << QByteArray("method=GET|path=/");
    QTest::newRow("query-string") << "/?message=bar" << int(QNetworkReply::NoError) << QByteArray("method=GET|path=/|get=bar");
    QTest::newRow("not-found") << "/not-found" << int(QNetworkReply::ContentNotFoundError) << errorTemplate.arg(QLatin1String("The document you requested was not found.")).toUtf8();
    QTest::newRow("internal-server-error") << "/internal-server-error" << internalServerError << errorTemplate.arg(QLatin1String("An internal server error was encountered.")).toUtf8();
}

void tst_QDjangoHttpServer::testGet()
{
    QFETCH(QString, path);
    QFETCH(int, err);
    QFETCH(QByteArray, body);

    QNetworkAccessManager network;
    QNetworkReply *reply = network.get(QNetworkRequest(QUrl(QLatin1String("http://127.0.0.1:8123") + path)));

    QEventLoop loop;
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    QVERIFY(reply);
    QCOMPARE(int(reply->error()), err);
    QCOMPARE(reply->readAll(), body);
    delete reply;
}

void tst_QDjangoHttpServer::testPost_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<int>("err");
    QTest::addColumn<QByteArray>("body");

    QTest::newRow("empty") << "/" << QByteArray() << int(QNetworkReply::NoError) << QByteArray("method=POST|path=/");
    QTest::newRow("simple") << "/" << QByteArray("message=bar") << int(QNetworkReply::NoError) << QByteArray("method=POST|path=/|post=bar");
    QTest::newRow("multi") << "/" << QByteArray("bob=wiz&message=bar&zoo=wow") << int(QNetworkReply::NoError) << QByteArray("method=POST|path=/|post=bar");
}

void tst_QDjangoHttpServer::testPost()
{
    QFETCH(QString, path);
    QFETCH(QByteArray, data);
    QFETCH(int, err);
    QFETCH(QByteArray, body);

    QNetworkAccessManager network;
    QNetworkRequest req(QUrl(QLatin1String("http://127.0.0.1:8123") + path));
    req.setRawHeader("Content-Type", "application/x-www-form-urlencoded");
    QNetworkReply *reply = network.post(req, data);

    QEventLoop loop;
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    QVERIFY(reply);
    QCOMPARE(int(reply->error()), err);
    QCOMPARE(reply->readAll(), body);
    delete reply;
}

QDjangoHttpResponse *tst_QDjangoHttpServer::_q_index(const QDjangoHttpRequest &request)
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

QDjangoHttpResponse *tst_QDjangoHttpServer::_q_error(const QDjangoHttpRequest &request)
{
    Q_UNUSED(request);

    return QDjangoHttpController::serveInternalServerError(request);
}

QTEST_MAIN(tst_QDjangoHttpServer)
#include "tst_qdjangohttpserver.moc"
