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

#include <QDateTime>
#include <QtTest>

#include "QDjangoHttpController.h"

/** Test QDjangoHttpController class.
 */
class tst_QDjangoHttpController : public QObject
{
    Q_OBJECT

private slots:
    void testDateTime();
};

void tst_QDjangoHttpController::testDateTime()
{
    const QDateTime dt(QDate(2014, 7, 14), QTime(11, 22, 33), Qt::UTC);
    QCOMPARE(QDjangoHttpController::httpDateTime(dt), QString("Mon, 14 Jul 2014 11:22:33 GMT"));
    QCOMPARE(QDjangoHttpController::httpDateTime("Mon, 14 Jul 2014 11:22:33 GMT"), dt);
}

QTEST_MAIN(tst_QDjangoHttpController)
#include "tst_qdjangohttpcontroller.moc"
