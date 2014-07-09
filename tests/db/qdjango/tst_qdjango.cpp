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

#include "QDjango.h"

#include "util.h"

class tst_QDjango : public QObject
{
    Q_OBJECT

private slots:
    void database();
    void debugEnabled();
    void debugQuery();
};

void tst_QDjango::database()
{
    QCOMPARE(QDjango::database().isOpen(), false);
    QVERIFY(initialiseDatabase());
    QCOMPARE(QDjango::database().isOpen(), true);
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
    QTest::ignoreMessage(QtDebugMsg, "SQL query \"SELECT foo\"");
    QVERIFY(!query.exec("SELECT foo"));
    QDjango::setDebugEnabled(false);
}

QTEST_MAIN(tst_QDjango)
#include "tst_qdjango.moc"
