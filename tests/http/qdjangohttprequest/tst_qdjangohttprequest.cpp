/*
 * Copyright (C) 2010-2013 Jeremy Lainé
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
    void testGet();
    void testPost();
};

void tst_QDjangoHttpRequest::testGet()
{
    QDjangoHttpRequest request;
    request.d->meta.insert("QUERY_STRING", "foo=bar");
    QCOMPARE(request.get(QLatin1String("foo")), QLatin1String("bar"));
}

void tst_QDjangoHttpRequest::testPost()
{
    QDjangoHttpRequest request;
    request.d->buffer = QByteArray("foo=bar");
    QCOMPARE(request.post(QLatin1String("foo")), QLatin1String("bar"));
}

QTEST_MAIN(tst_QDjangoHttpRequest)
#include "tst_qdjangohttprequest.moc"
