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

    QCOMPARE(int(reply->error()), err);
    QCOMPARE(reply->readAll(), body);
    delete reply;
}

