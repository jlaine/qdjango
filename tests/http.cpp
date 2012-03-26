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

class TestController : public QDjangoHttpController
{
    QDjangoHttpResponse *respondToRequest(const QDjangoHttpRequest &request);
};

QDjangoHttpResponse *TestController::respondToRequest(const QDjangoHttpRequest &request)
{
    if (request.path() == "/") {
        QDjangoHttpResponse *response = new QDjangoHttpResponse;
        response->setHeader("Content-Type", "text/plain");
        response->setBody("hello");
        return response;
    } else if (request.path() == "/internal-server-error") {
        return serveInternalServerError(request);
    }
    return serveNotFound(request);
}

void TestHttp::cleanupTestCase()
{
    delete httpServer;
    delete httpController;
}

void TestHttp::initTestCase()
{
    httpController = new TestController;
    httpServer = new QDjangoHttpServer;
    httpServer->setController(httpController);
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

    QTest::newRow("root") << "/" << int(QNetworkReply::NoError) << QByteArray("hello");
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

void tst_QDjangoUrlResolver::cleanupTestCase()
{
    delete urlResolver;
}

void tst_QDjangoUrlResolver::initTestCase()
{
    urlResolver = new QDjangoUrlResolver;
    QVERIFY(urlResolver->addView(QRegExp("^/$"), this, "_q_index"));
    QVERIFY(urlResolver->addView(QRegExp("^/test/$"), this, "_q_noArgs"));
    QVERIFY(urlResolver->addView(QRegExp("^/test/([0-9]+)/$"), this, "_q_oneArg"));
    QVERIFY(urlResolver->addView(QRegExp("^/test/([0-9]+)/([a-z]+)/$"), this, "_q_twoArgs"));
}

void tst_QDjangoUrlResolver::testRespond_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<int>("err");

    QTest::newRow("root") << "/" << 200;
    QTest::newRow("not-found") << "/foo/" << 404;
    QTest::newRow("no-args") << "/test/" << 200;
    QTest::newRow("one-args") << "/test/123/" << 200;
    QTest::newRow("two-args") << "/test/123/delete/" << 200;
    QTest::newRow("three-args") << "/test/123/delete/zoo/" << 404;
}

void tst_QDjangoUrlResolver::testRespond()
{
    QFETCH(QString, path);
    QFETCH(int, err);

    QDjangoHttpResponse *response = urlResolver->respond(QDjangoHttpTestRequest("GET", path));
    QVERIFY(response);
    QCOMPARE(int(response->statusCode()), err);
}

void tst_QDjangoUrlResolver::testReverse_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<QString>("member");
    QTest::addColumn<QString>("args");

    QTest::newRow("root") << "/" << "_q_index" << "";
    QTest::newRow("no-args") << "/test/" << "_q_noArgs" << "";
    QTest::newRow("one-arg") << "/test/123/" << "_q_oneArg" << "123";
    QTest::newRow("two-args") << "/test/123/delete/" << "_q_twoArgs" << "123|delete";
    QTest::newRow("too-few-args") << "" << "_q_oneArg" << "";
    QTest::newRow("too-many-args") << "" << "_q_noArgs" << "123";
}

void tst_QDjangoUrlResolver::testReverse()
{
    QFETCH(QString, path);
    QFETCH(QString, member);
    QFETCH(QString, args);

    QVariantList varArgs;
    if (!args.isEmpty()) {
        foreach (const QString &bit, args.split('|'))
            varArgs << bit;
    }
    QCOMPARE(urlResolver->reverse(this, member.toLatin1(), varArgs), path);
}

QDjangoHttpResponse* tst_QDjangoUrlResolver::_q_index(const QDjangoHttpRequest &request)
{
    return new QDjangoHttpResponse;
}

QDjangoHttpResponse* tst_QDjangoUrlResolver::_q_noArgs(const QDjangoHttpRequest &request)
{
    return new QDjangoHttpResponse;
}

QDjangoHttpResponse* tst_QDjangoUrlResolver::_q_oneArg(const QDjangoHttpRequest &request, const QString &id)
{
    return new QDjangoHttpResponse;
}

QDjangoHttpResponse* tst_QDjangoUrlResolver::_q_twoArgs(const QDjangoHttpRequest &request, const QString &id, const QString &action)
{
    return new QDjangoHttpResponse;
}
