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

#include <QtTest>

#include "QDjangoHttpResponse.h"

/** Test QDjangoHttpServer class.
 */
class tst_QDjangoHttpResponse : public QObject
{
    Q_OBJECT

private slots:
    void testBody();
    void testHeader();
    void testStatusCode_data();
    void testStatusCode();
};

void tst_QDjangoHttpResponse::testBody()
{
    QDjangoHttpResponse response;
    QCOMPARE(response.body(), QByteArray());

    response.setBody("foo=bar");
    QCOMPARE(response.body(), QByteArray("foo=bar"));
}

void tst_QDjangoHttpResponse::testHeader()
{
    QDjangoHttpResponse response;
    QCOMPARE(response.header("Content-Type"), QString());

    response.setHeader("Content-Type", "application/json");
    QCOMPARE(response.header("Content-Type"), QString("application/json"));
    QCOMPARE(response.header("content-type"), QString("application/json"));
}

void tst_QDjangoHttpResponse::testStatusCode_data()
{
    QTest::addColumn<int>("statusCode");
    QTest::addColumn<QString>("reasonPhrase");

    QTest::newRow("200") << int(200) << QString("OK");
    QTest::newRow("301") << int(301) << QString("Moved Permanently");
    QTest::newRow("302") << int(302) << QString("Found");
    QTest::newRow("304") << int(304) << QString("Not Modified");
    QTest::newRow("400") << int(400) << QString("Bad Request");
    QTest::newRow("401") << int(401) << QString("Authorization Required");
    QTest::newRow("403") << int(403) << QString("Forbidden");
    QTest::newRow("403") << int(404) << QString("Not Found");
    QTest::newRow("405") << int(405) << QString("Method Not Allowed");
    QTest::newRow("500") << int(500) << QString("Internal Server Error");
    QTest::newRow("501") << int(501) << QString();
}

void tst_QDjangoHttpResponse::testStatusCode()
{
    QFETCH(int, statusCode);
    QFETCH(QString, reasonPhrase);

    QDjangoHttpResponse response;
    response.setStatusCode(statusCode);
    QCOMPARE(response.statusCode(), statusCode);
    QCOMPARE(response.reasonPhrase(), reasonPhrase);
}

QTEST_MAIN(tst_QDjangoHttpResponse)
#include "tst_qdjangohttpresponse.moc"
