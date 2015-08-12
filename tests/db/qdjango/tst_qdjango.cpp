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

#include <QSqlDriver>
#include <QThread>
#include <QTimer>

#include "QDjango.h"
#include "QDjangoModel.h"
#include "QDjangoQuerySet.h"

#include "util.h"

class Author : public QDjangoModel
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName)

public:
    Author(QObject *parent = 0) : QDjangoModel(parent) {}

    QString name() const { return m_name; }
    void setName(const QString &name) { m_name = name; }

private:
    QString m_name;
};

class Worker : public QObject
{
    Q_OBJECT

public slots:
    void doIt();

signals:
    void done();
};

void Worker::doIt()
{
    Author author;
    author.setName("someone");
    QVERIFY(author.save());

    emit done();
}

class tst_QDjango : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void init();
    void databaseThreaded();
    void debugEnabled();
    void debugQuery();
    void cleanup();
};

void tst_QDjango::initTestCase()
{
    QCOMPARE(QDjango::database().isOpen(), false);
    QVERIFY(initialiseDatabase());
    QCOMPARE(QDjango::database().isOpen(), true);
}

void tst_QDjango::init()
{
    QDjango::registerModel<Author>();

    QSqlDatabase db = QDjango::database();
    QVERIFY(db.tables().indexOf("author") == -1);
    QVERIFY(QDjango::createTables());
    QVERIFY(db.tables().indexOf("author") != -1);
}

void tst_QDjango::cleanup()
{
    QSqlDatabase db = QDjango::database();
    QVERIFY(QDjango::dropTables());
    QVERIFY(db.tables().indexOf("author") == -1);
}

void tst_QDjango::databaseThreaded()
{
    if (QDjango::database().databaseName() == QLatin1String(":memory:"))
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
        QSKIP("Threaded test cannot work with in-memory SQLite database.");
#else
        QSKIP("Threaded test cannot work with in-memory SQLite database.", SkipAll);
#endif

    QDjangoQuerySet<Author> qs;
    QCOMPARE(qs.count(), 0);

    QEventLoop loop;
    Worker worker;
    QThread workerThread;

    // fire up worker
    worker.moveToThread(&workerThread);
    connect(&worker, SIGNAL(done()), &loop, SLOT(quit()));

    workerThread.start();
    QTimer::singleShot(0, &worker, SLOT(doIt()));
    loop.exec();

    // check database
    QCOMPARE(qs.count(), 1);

    // stop thread
    workerThread.quit();
    workerThread.wait();
}

void tst_QDjango::debugEnabled()
{
    QCOMPARE(QDjango::isDebugEnabled(), false);
    QDjango::setDebugEnabled(true);
    QCOMPARE(QDjango::isDebugEnabled(), true);
    QDjango::setDebugEnabled(false);
    QCOMPARE(QDjango::isDebugEnabled(), false);
}

void tst_QDjango::debugQuery()
{
    QDjangoQuery query(QDjango::database());
    QDjango::setDebugEnabled(true);
    QVERIFY(!query.exec("SELECT foo"));
    QDjango::setDebugEnabled(false);
}

QTEST_MAIN(tst_QDjango)
#include "tst_qdjango.moc"
