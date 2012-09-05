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

#include "http.h"

void TestHttp::cleanupTestCase()
{
    delete httpServer;
}

void TestHttp::initTestCase()
{
    httpServer = new QDjangoHttpServer;
    httpServer->urls()->set(QRegExp("^$"), this, "_q_index");
    httpServer->urls()->set(QRegExp("^internal-server-error$"), this, "_q_error");
    QCOMPARE(httpServer->listen(QHostAddress::LocalHost, 8123), true);
}

void TestHttp::testGet_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<int>("err");
    QTest::addColumn<QByteArray>("body");

    const QString errorTemplate(
        "<html>"
        "<head><title>Error</title></head>"
        "<body><p>%1</p></body>"
        "</html>");

    QTest::newRow("root") << "/" << int(QNetworkReply::NoError) << QByteArray("method=GET|path=/");
    QTest::newRow("query-string") << "/?message=bar" << int(QNetworkReply::NoError) << QByteArray("method=GET|path=/|get=bar");
    QTest::newRow("not-found") << "/not-found" << int(QNetworkReply::ContentNotFoundError) << errorTemplate.arg("The document you requested was not found.").toUtf8();
    QTest::newRow("internal-server-error") << "/internal-server-error" << int(QNetworkReply::UnknownContentError) << errorTemplate.arg("An internal server error was encountered.").toUtf8();
}

void TestHttp::testGet()
{
    QFETCH(QString, path);
    QFETCH(int, err);
    QFETCH(QByteArray, body);

    QNetworkAccessManager network;
    QNetworkReply *reply = network.get(QNetworkRequest(QUrl("http://127.0.0.1:8123" + path)));

    QEventLoop loop;
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    QVERIFY(reply);
    QCOMPARE(int(reply->error()), err);
    QCOMPARE(reply->readAll(), body);
    delete reply;
}

void TestHttp::testPost_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<int>("err");
    QTest::addColumn<QByteArray>("body");

    QTest::newRow("empty") << "/" << QByteArray() << int(QNetworkReply::NoError) << QByteArray("method=POST|path=/");
    QTest::newRow("simple") << "/" << QByteArray("message=bar") << int(QNetworkReply::NoError) << QByteArray("method=POST|path=/|post=bar");
    QTest::newRow("multi") << "/" << QByteArray("bob=wiz&message=bar&zoo=wow") << int(QNetworkReply::NoError) << QByteArray("method=POST|path=/|post=bar");
}

void TestHttp::testPost()
{
    QFETCH(QString, path);
    QFETCH(QByteArray, data);
    QFETCH(int, err);
    QFETCH(QByteArray, body);

    QNetworkAccessManager network;
    QNetworkRequest req(QUrl("http://127.0.0.1:8123" + path));
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

QDjangoHttpResponse *TestHttp::_q_index(const QDjangoHttpRequest &request)
{
    QDjangoHttpResponse *response = new QDjangoHttpResponse;
    response->setHeader("Content-Type", "text/plain");

    QString output = "method=" + request.method();
    output += "|path=" + request.path();

    const QString getValue = request.get("message");
    if (!getValue.isEmpty())
        output += "|get=" + getValue;

    const QString postValue = request.post("message");
    if (!postValue.isEmpty())
        output += "|post=" + postValue;

    response->setBody(output.toUtf8());
    return response;
}

QDjangoHttpResponse *TestHttp::_q_error(const QDjangoHttpRequest &request)
{
    Q_UNUSED(request);

    return QDjangoHttpController::serveInternalServerError(request);
}

QDjangoHttpResponse* tst_QDjangoUrlHelper::_q_index(const QDjangoHttpRequest &request)
{
    Q_UNUSED(request);

    QDjangoHttpResponse *response = new QDjangoHttpResponse;
    response->setHeader("Content-Type", "text/plain");
    response->setBody("sub index");
    return response;
}

QDjangoHttpResponse* tst_QDjangoUrlHelper::_q_test(const QDjangoHttpRequest &request)
{
    Q_UNUSED(request);

    QDjangoHttpResponse *response = new QDjangoHttpResponse;
    response->setHeader("Content-Type", "text/plain");
    response->setBody("sub test");
    return response;
}

void tst_QDjangoUrlResolver::cleanupTestCase()
{
    delete urlResolver;
}

void tst_QDjangoUrlResolver::initTestCase()
{
    urlHelper = new tst_QDjangoUrlHelper;
    urlSub = new QDjangoUrlResolver;
    QVERIFY(urlSub->set(QRegExp("^$"), urlHelper, "_q_index"));
    QVERIFY(urlSub->set(QRegExp("^test/$"), urlHelper, "_q_test"));

    urlResolver = new QDjangoUrlResolver;
    QVERIFY(urlResolver->set(QRegExp("^$"), this, "_q_index"));
    QVERIFY(urlResolver->set(QRegExp("^test/$"), this, "_q_noArgs"));
    QVERIFY(urlResolver->set(QRegExp("^test/([0-9]+)/$"), this, "_q_oneArg"));
    QVERIFY(urlResolver->set(QRegExp("^test/([0-9]+)/([a-z]+)/$"), this, "_q_twoArgs"));
    QVERIFY(urlResolver->include(QRegExp("^recurse/"), urlSub));
}

void tst_QDjangoUrlResolver::testRespond_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<int>("err");
    QTest::addColumn<QString>("body");

    QTest::newRow("root") << "/" << 200 << "";
    QTest::newRow("not-found") << "/non-existent/" << 404 << "";
    QTest::newRow("no-args") << "/test/" << 200 << "";
    QTest::newRow("one-args") << "/test/123/" << 200 << "";
    QTest::newRow("two-args") << "/test/123/delete/" << 200 << "";
    QTest::newRow("three-args") << "/test/123/delete/zoo/" << 404 << "";
    QTest::newRow("recurse-not-found") << "/recurse/non-existent/" << 404 << "";
    QTest::newRow("recurse-index") << "/recurse/" << 200 << "";
    QTest::newRow("recurse-test") << "/recurse/test/" << 200 << "";
}

void tst_QDjangoUrlResolver::testRespond()
{
    QFETCH(QString, path);
    QFETCH(int, err);
    QFETCH(QString, body);

    QDjangoHttpResponse *response = urlResolver->respond(QDjangoHttpTestRequest("GET", path), path);
    QVERIFY(response);
    QCOMPARE(int(response->statusCode()), err);
}

void tst_QDjangoUrlResolver::testReverse_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<QObject*>("receiver");
    QTest::addColumn<QString>("member");
    QTest::addColumn<QString>("args");

    QObject *receiver = this;
    QTest::newRow("root") << "/" << receiver << "_q_index" << "";
    QTest::newRow("no-args") << "/test/" << receiver << "_q_noArgs" << "";
    QTest::newRow("one-arg") << "/test/123/" << receiver << "_q_oneArg" << "123";
    QTest::newRow("two-args") << "/test/123/delete/" << receiver << "_q_twoArgs" << "123|delete";
    QTest::newRow("too-few-args") << "" << receiver << "_q_oneArg" << "";
    QTest::newRow("too-many-args") << "" << receiver << "_q_noArgs" << "123";

    receiver = urlHelper;
    QTest::newRow("recurse-index") << "/recurse/" << receiver << "_q_index" << "";
    QTest::newRow("recurse-test") << "/recurse/test/" << receiver << "_q_test" << "";
}

void tst_QDjangoUrlResolver::testReverse()
{
    QFETCH(QString, path);
    QFETCH(QObject*, receiver);
    QFETCH(QString, member);
    QFETCH(QString, args);

    QVariantList varArgs;
    if (!args.isEmpty()) {
        foreach (const QString &bit, args.split('|'))
            varArgs << bit;
    }
    QCOMPARE(urlResolver->reverse(receiver, member.toLatin1(), varArgs), path);
}

QDjangoHttpResponse* tst_QDjangoUrlResolver::_q_index(const QDjangoHttpRequest &request)
{
    Q_UNUSED(request);

    return new QDjangoHttpResponse;
}

QDjangoHttpResponse* tst_QDjangoUrlResolver::_q_noArgs(const QDjangoHttpRequest &request)
{
    Q_UNUSED(request);

    return new QDjangoHttpResponse;
}

QDjangoHttpResponse* tst_QDjangoUrlResolver::_q_oneArg(const QDjangoHttpRequest &request, const QString &id)
{
    Q_UNUSED(request);
    Q_UNUSED(id);

    return new QDjangoHttpResponse;
}

QDjangoHttpResponse* tst_QDjangoUrlResolver::_q_twoArgs(const QDjangoHttpRequest &request, const QString &id, const QString &action)
{
    Q_UNUSED(request);
    Q_UNUSED(id);
    Q_UNUSED(action);

    return new QDjangoHttpResponse;
}
