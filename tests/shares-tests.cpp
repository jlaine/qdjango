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

#include <QtTest/QtTest>

#include "QDjangoQuerySet.h"

#include "shares-models.h"
#include "shares-tests.h"

/** Create database table before running tests.
 */
void TestShares::initTestCase()
{
    QCOMPARE(QDjango::registerModel<File>().createTable(), true);
}

void TestShares::testFile()
{
    // create a file
    File file;
    file.setDate(QDateTime(QDate(2010, 6, 1), QTime(10, 5, 14)));
    file.setHash(QByteArray("\0\1\2\3\4", 5));
    file.setPath("foo/bar.txt");
    file.setSize(1234);
    QCOMPARE(file.save(), true);

    File *other = QDjangoQuerySet<File>().get(QDjangoWhere("path", QDjangoWhere::Equals, "foo/bar.txt"));
    QVERIFY(other != 0);
    QCOMPARE(other->date(), QDateTime(QDate(2010, 6, 1), QTime(10, 5, 14)));
    QCOMPARE(other->hash(), QByteArray("\0\1\2\3\4", 5));
    QCOMPARE(other->path(), QLatin1String("foo/bar.txt"));
    QCOMPARE(other->size(), qint64(1234));
    delete other;

    // update the file
    file.setSize(5678);
    QCOMPARE(file.save(), true);
}

/** Clear database table after each test.
 */
void TestShares::cleanup()
{
    QCOMPARE(QDjangoQuerySet<File>().remove(), true);
}

/** Drop database table after running tests.
 */
void TestShares::cleanupTestCase()
{
    QCOMPARE(QDjango::registerModel<File>().dropTable(), true);
}

