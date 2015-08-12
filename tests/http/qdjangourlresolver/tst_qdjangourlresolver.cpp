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
#include "QDjangoUrlResolver.h"

class tst_QDjangoUrlHelper : public QObject
{
    Q_OBJECT

private slots:
    QDjangoHttpResponse* _q_index(const QDjangoHttpRequest &request);
    QDjangoHttpResponse* _q_test(const QDjangoHttpRequest &request);
};

class tst_QDjangoUrlResolver : public QObject
{
    Q_OBJECT

private slots:
    void cleanupTestCase();
    void initTestCase();
    void testRespond_data();
    void testRespond();
    void testReverse_data();
    void testReverse();

    QDjangoHttpResponse* _q_index(const QDjangoHttpRequest &request);
    QDjangoHttpResponse* _q_noArgs(const QDjangoHttpRequest &request);
    QDjangoHttpResponse* _q_oneArg(const QDjangoHttpRequest &request, const QString &id);
    QDjangoHttpResponse* _q_twoArgs(const QDjangoHttpRequest &request, const QString &id, const QString &action);

private:
    tst_QDjangoUrlHelper *urlHelper;
    QDjangoUrlResolver *urlResolver;
    QDjangoUrlResolver *urlSub;
};

QDjangoHttpResponse* tst_QDjangoUrlHelper::_q_index(const QDjangoHttpRequest &request)
{
    Q_UNUSED(request);

    QDjangoHttpResponse *response = new QDjangoHttpResponse;
    response->setHeader(QLatin1String("Content-Type"), QLatin1String("text/plain"));
    response->setBody("sub index");
    return response;
}

QDjangoHttpResponse* tst_QDjangoUrlHelper::_q_test(const QDjangoHttpRequest &request)
{
    Q_UNUSED(request);

    QDjangoHttpResponse *response = new QDjangoHttpResponse;
    response->setHeader(QLatin1String("Content-Type"), QLatin1String("text/plain"));
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
    QVERIFY(urlSub->set(QRegExp(QLatin1String("^$")), urlHelper, "_q_index"));
    QVERIFY(urlSub->set(QRegExp(QLatin1String("^test/$")), urlHelper, "_q_test"));

    urlResolver = new QDjangoUrlResolver;
    QVERIFY(urlResolver->set(QRegExp(QLatin1String("^$")), this, "_q_index"));
    QVERIFY(urlResolver->set(QRegExp(QLatin1String("^test/$")), this, "_q_noArgs"));
    QVERIFY(urlResolver->set(QRegExp(QLatin1String("^test/([0-9]+)/$")), this, "_q_oneArg"));
    QVERIFY(urlResolver->set(QRegExp(QLatin1String("^test/([0-9]+)/([a-z]+)/$")), this, "_q_twoArgs"));
    QVERIFY(urlResolver->include(QRegExp(QLatin1String("^recurse/")), urlSub));
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

    QDjangoHttpTestRequest request(QLatin1String("GET"), path);
    QDjangoHttpResponse *response = urlResolver->respond(request, path);
    QVERIFY(response);
    QCOMPARE(int(response->statusCode()), err);
}

void tst_QDjangoUrlResolver::testReverse_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<QObject*>("receiver");
    QTest::addColumn<QString>("member");
    QTest::addColumn<QString>("args");
    QTest::addColumn<QString>("warning");

    QObject *receiver = this;
    QTest::newRow("root") << "/" << receiver << "_q_index" << "" << "";
    QTest::newRow("no-args") << "/test/" << receiver << "_q_noArgs" << "" << "";
    QTest::newRow("one-arg") << "/test/123/" << receiver << "_q_oneArg" << "123" << "";
    QTest::newRow("two-args") << "/test/123/delete/" << receiver << "_q_twoArgs" << "123|delete" << "";
    QTest::newRow("too-few-args") << "" << receiver << "_q_oneArg" << "" << "Too few arguments for '_q_oneArg'";
    QTest::newRow("too-many-args") << "" << receiver << "_q_noArgs" << "123" << "Too many arguments for '_q_noArgs'";

    receiver = urlHelper;
    QTest::newRow("recurse-index") << "/recurse/" << receiver << "_q_index" << "" << "";
    QTest::newRow("recurse-test") << "/recurse/test/" << receiver << "_q_test" << "" << "";
}

void tst_QDjangoUrlResolver::testReverse()
{
    QFETCH(QString, path);
    QFETCH(QObject*, receiver);
    QFETCH(QString, member);
    QFETCH(QString, args);
    QFETCH(QString, warning);

    QVariantList varArgs;
    if (!args.isEmpty()) {
        foreach (const QString &bit, args.split(QLatin1Char('|')))
            varArgs << bit;
    }
    if (!warning.isEmpty())
        QTest::ignoreMessage(QtWarningMsg, warning.toLatin1());
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

QTEST_MAIN(tst_QDjangoUrlResolver)
#include "tst_qdjangourlresolver.moc"
