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
    QDjangoQuerySet<Author> qs;
    Worker worker;
    QThread workerThread;

    worker.moveToThread(&workerThread);
    connect(&worker, SIGNAL(done()), &workerThread, SLOT(quit()));
    QTimer::singleShot(0, &worker, SLOT(doIt()));

    QEventLoop loop;
    QObject::connect(&workerThread, SIGNAL(finished()), &loop, SLOT(quit()));

    workerThread.start();
    loop.exec();

    QCOMPARE(qs.count(), 1);
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
