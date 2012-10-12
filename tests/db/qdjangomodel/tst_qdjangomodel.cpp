/*
 * Copyright (C) 2010-2012 Jeremy Lainé
 * Contact: http://code.google.com/p/qdjango/
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

#include "QDjango.h"
#include "QDjangoModel.h"
#include "QDjangoQuerySet.h"
#include "QDjangoWhere.h"
#include "util.h"

class TestModel : public QDjangoModel
{
public:
    TestModel(QObject *parent = 0) : QDjangoModel(parent) {}

    // expose foreign key methods so they can be tested
    QObject *foreignKey(const char *name) const
    { return QDjangoModel::foreignKey(name); }
    void setForeignKey(const char *name, QObject *value)
    { QDjangoModel::setForeignKey(name, value); }
};

class Author : public TestModel
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName)

public:
    Author(QObject *parent = 0) : TestModel(parent) {}

    QString name() const { return m_name; }
    void setName(const QString &name) { m_name = name; }

private:
    QString m_name;
};

class Book : public TestModel
{
    Q_OBJECT
    Q_PROPERTY(Author* author READ author WRITE setAuthor)
    Q_PROPERTY(QString title READ title WRITE setTitle)

public:
    Book(QObject *parent = 0)
        : TestModel(parent)
    {
        setForeignKey("author", new Author(this));
    }

    Author *author() const { return qobject_cast<Author*>(foreignKey("author")); }
    void setAuthor(Author *author) { setForeignKey("author", author); }

    QString title() const { return m_title; }
    void setTitle(const QString &title) { m_title = title; }

private:
    QString m_title;
};

/** Test QDjangoModel class.
 */
class tst_QDjangoModel : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void init();
    void foreignKey();
    void setForeignKey();
    void filterRelated();
    void selectRelated();
    void cleanup();
    void cleanupTestCase();
};

/** Create database tables before running tests.
 */
void tst_QDjangoModel::initTestCase()
{
    QVERIFY(initialiseDatabase());
    QCOMPARE(QDjango::registerModel<Author>().createTable(), true);
    QCOMPARE(QDjango::registerModel<Book>().createTable(), true);
}

/** Load fixtures.
 */
void tst_QDjangoModel::init()
{
    Author author1;
    author1.setName("First author");
    QCOMPARE(author1.save(), true);

    Author author2;
    author2.setName("Second author");
    QCOMPARE(author2.save(), true);

    Book book;
    book.setAuthor(&author1);
    book.setTitle("Some book");
    QCOMPARE(book.save(), true);

    Book book2;
    book2.setAuthor(&author2);
    book2.setTitle("Other book");
    QCOMPARE(book2.save(), true);
}

void tst_QDjangoModel::foreignKey()
{
    QTest::ignoreMessage(QtWarningMsg, "QDjangoMetaModel cannot get foreign model for invalid key 'bad'");
    Book book;
    QVERIFY(!book.foreignKey("bad"));
    QVERIFY(book.foreignKey("author"));
}

void tst_QDjangoModel::setForeignKey()
{
    QTest::ignoreMessage(QtWarningMsg, "QDjangoMetaModel cannot set foreign model for invalid key 'bad'");
    Book book;
    book.setForeignKey("bad", 0);
    book.setForeignKey("author", 0);
}

/** Perform filtering on foreign keys.
 */
void tst_QDjangoModel::filterRelated()
{
    QDjangoQuerySet<Book> books;

    QDjangoQuerySet<Book> qs = books.filter(
        QDjangoWhere("author__name", QDjangoWhere::Equals, "First author"));
    CHECKWHERE(qs.where(), QLatin1String("T0.\"name\" = ?"), QVariantList() << "First author");
    QCOMPARE(qs.count(), 1);
    QCOMPARE(qs.size(), 1);

    Book *book = qs.at(0);
    QVERIFY(book != 0);
    QCOMPARE(book->title(), QLatin1String("Some book"));
    delete book;
}

/** Test eager loading of foreign keys.
 */
void tst_QDjangoModel::selectRelated()
{
    // without eager loading
    QDjangoQuerySet<Book> qs;
    Book *book = qs.get(QDjangoWhere("title", QDjangoWhere::Equals, "Some book"));
    QVERIFY(book != 0);
    QCOMPARE(book->title(), QLatin1String("Some book"));
    QVERIFY(book->author() != 0);
    QCOMPARE(book->author()->name(), QLatin1String("First author"));
    delete book;

    // with eager loading
    book = qs.selectRelated().get(QDjangoWhere("title", QDjangoWhere::Equals, "Some book"));
    QVERIFY(book != 0);
    QCOMPARE(book->title(), QLatin1String("Some book"));
    QVERIFY(book->author() != 0);
    QCOMPARE(book->author()->name(), QLatin1String("First author"));
    delete book;
}

/** Clear database tables after each test.
 */
void tst_QDjangoModel::cleanup()
{
    QCOMPARE(QDjangoQuerySet<Book>().remove(), true);
    QCOMPARE(QDjangoQuerySet<Author>().remove(), true);
}

/** Drop database tables after running tests.
 */
void tst_QDjangoModel::cleanupTestCase()
{
    QCOMPARE(QDjango::registerModel<Book>().dropTable(), true);
    QCOMPARE(QDjango::registerModel<Author>().dropTable(), true);
}

QTEST_MAIN(tst_QDjangoModel)
#include "tst_qdjangomodel.moc"
