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

#include "QDjangoQuerySet.h"

#include "util.h"

/** Tests for the File class.
 */
class tst_Shares : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void testFile();
    void cleanup();
    void cleanupTestCase();
};

class File : public QDjangoModel
{
    Q_OBJECT
    Q_PROPERTY(QDateTime date READ date WRITE setDate)
    Q_PROPERTY(QByteArray hash READ hash WRITE setHash)
    Q_PROPERTY(QString path READ path WRITE setPath)
    Q_PROPERTY(qint64 size READ size WRITE setSize)

    Q_CLASSINFO("path", "max_length=255 primary_key=true")
    Q_CLASSINFO("hash", "max_length=32")

public:
    File(QObject *parent = 0);

    QDateTime date() const;
    void setDate(const QDateTime &date);

    QByteArray hash() const;
    void setHash(const QByteArray &hash);

    QString path() const;
    void setPath(const QString &path);

    qint64 size() const;
    void setSize(qint64 size);

private:
    QDateTime m_date;
    QByteArray m_hash;
    QString m_path;
    qint64 m_size;
};

File::File(QObject *parent)
    : QDjangoModel(parent), m_size(0)
{
}

QDateTime File::date() const
{
    return m_date;
}

void File::setDate(const QDateTime &date)
{
    m_date = date;
}

QByteArray File::hash() const
{
    return m_hash;
}

void File::setHash(const QByteArray &hash)
{
    m_hash = hash;
}

QString File::path() const
{
    return m_path;
}

void File::setPath(const QString &path)
{
    m_path = path;
}

qint64 File::size() const
{
    return m_size;
}

void File::setSize(qint64 size)
{
    m_size = size;
}

/** Create database table before running tests.
 */
void tst_Shares::initTestCase()
{
    QVERIFY(initialiseDatabase());
    QDjango::registerModel<File>();
    QVERIFY(QDjango::createTables());
}

void tst_Shares::testFile()
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
void tst_Shares::cleanup()
{
    QCOMPARE(QDjangoQuerySet<File>().remove(), true);
}

/** Drop database table after running tests.
 */
void tst_Shares::cleanupTestCase()
{
    QVERIFY(QDjango::dropTables());
}

QTEST_MAIN(tst_Shares)
#include "tst_shares.moc"
