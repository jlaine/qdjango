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

#include "QDjangoHttpRequest.h"
#include "QDjangoHttpRequest_p.h"

/** Test QDjangoHttpServer class.
 */
class tst_QDjangoHttpRequest : public QObject
{
    Q_OBJECT

private slots:
    void testBody();
    void testGet();
    void testPost();
};

void tst_QDjangoHttpRequest::testBody()
{
    QDjangoHttpRequest request;

    QCOMPARE(request.body(), QByteArray());

    request.d->buffer = QByteArray("foo=bar");
    QCOMPARE(request.body(), QByteArray("foo=bar"));
}

void tst_QDjangoHttpRequest::testGet()
{
    QDjangoHttpRequest request;

    // plain
    request.d->meta.insert("QUERY_STRING", "foo=bar&baz=qux");
    QCOMPARE(request.get(QLatin1String("foo")), QLatin1String("bar"));
    QCOMPARE(request.get(QLatin1String("baz")), QLatin1String("qux"));

    // space encoded as plus
    request.d->meta.insert("QUERY_STRING", "foo=bar+more&baz=qux");
    QCOMPARE(request.get(QLatin1String("foo")), QLatin1String("bar more"));
    QCOMPARE(request.get(QLatin1String("baz")), QLatin1String("qux"));

    // plus encoded as %2B
    request.d->meta.insert("QUERY_STRING", "foo=bar%2Bmore&baz=qux");
    QCOMPARE(request.get(QLatin1String("foo")), QLatin1String("bar+more"));
    QCOMPARE(request.get(QLatin1String("baz")), QLatin1String("qux"));

    // plus encoded as %2b
    request.d->meta.insert("QUERY_STRING", "foo=bar%2bmore&baz=qux");
    QCOMPARE(request.get(QLatin1String("foo")), QLatin1String("bar+more"));
    QCOMPARE(request.get(QLatin1String("baz")), QLatin1String("qux"));

    // at encoded as %40
    request.d->meta.insert("QUERY_STRING", "foo=bar%40example.com&baz=qux");
    QCOMPARE(request.get(QLatin1String("foo")), QLatin1String("bar@example.com"));
    QCOMPARE(request.get(QLatin1String("baz")), QLatin1String("qux"));
}

void tst_QDjangoHttpRequest::testPost()
{
    QDjangoHttpRequest request;

    // plain
    request.d->buffer = QByteArray("foo=bar&baz=qux");
    QCOMPARE(request.post(QLatin1String("foo")), QLatin1String("bar"));
    QCOMPARE(request.post(QLatin1String("baz")), QLatin1String("qux"));

    // space encoded as plus
    request.d->buffer = QByteArray("foo=bar+more&baz=qux");
    QCOMPARE(request.post(QLatin1String("foo")), QLatin1String("bar more"));
    QCOMPARE(request.post(QLatin1String("baz")), QLatin1String("qux"));

    // plus encoded as %2B
    request.d->buffer = QByteArray("foo=bar%2Bmore&baz=qux");
    QCOMPARE(request.post(QLatin1String("foo")), QLatin1String("bar+more"));
    QCOMPARE(request.post(QLatin1String("baz")), QLatin1String("qux"));

    // plus encoded as %2b
    request.d->buffer = QByteArray("foo=bar%2bmore&baz=qux");
    QCOMPARE(request.post(QLatin1String("foo")), QLatin1String("bar+more"));
    QCOMPARE(request.post(QLatin1String("baz")), QLatin1String("qux"));

    // at encoded as %40
    request.d->buffer = QByteArray("foo=bar%40example.com&baz=qux");
    QCOMPARE(request.post(QLatin1String("foo")), QLatin1String("bar@example.com"));
    QCOMPARE(request.post(QLatin1String("baz")), QLatin1String("qux"));
}

QTEST_MAIN(tst_QDjangoHttpRequest)
#include "tst_qdjangohttprequest.moc"
