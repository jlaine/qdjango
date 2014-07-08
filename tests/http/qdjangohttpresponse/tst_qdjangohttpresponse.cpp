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

QTEST_MAIN(tst_QDjangoHttpResponse)
#include "tst_qdjangohttpresponse.moc"
