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
    Q_CLASSINFO("author", "on_delete=cascade")

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

class BookWithNullAuthor : public TestModel
{
    Q_OBJECT
    Q_PROPERTY(Author* author READ author WRITE setAuthor)
    Q_PROPERTY(QString title READ title WRITE setTitle)
    Q_CLASSINFO("author", "null=true on_delete=cascade")

public:
    BookWithNullAuthor(QObject *parent = 0) : TestModel(parent) {}

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
    void deleteCascade();
    void foreignKey();
    void foreignKey_null();
    void setForeignKey();
    void filterRelated();
    void filterRelatedReverse();
    void filterRelatedReverse_null();
    void primaryKey();
    void selectRelated();
    void selectRelated_null();
    void toString();
    void cleanup();
    void cleanupTestCase();
};

/** Create database tables before running tests.
 */
void tst_QDjangoModel::initTestCase()
{
    QVERIFY(initialiseDatabase());
    QDjango::registerModel<Author>();
    QDjango::registerModel<Book>();
    QDjango::registerModel<BookWithNullAuthor>();
}

/** Load fixtures.
 */
void tst_QDjangoModel::init()
{
    QVERIFY(QDjango::createTables());

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

    BookWithNullAuthor book3;
    book3.setTitle("Book with null author");
    QCOMPARE(book3.save(), true);

    BookWithNullAuthor book4;
    book4.setAuthor(&author1);
    book4.setTitle("Some book");
    QCOMPARE(book4.save(), true);
}

void tst_QDjangoModel::deleteCascade()
{
    const QDjangoQuerySet<Author> authors;
    const QDjangoQuerySet<Book> books;
    QCOMPARE(authors.count(), 2);
    QCOMPARE(books.count(), 2);

    QVERIFY(authors.filter(QDjangoWhere("name", QDjangoWhere::Equals, "First author")).remove());

    QCOMPARE(authors.count(), 1);
    QCOMPARE(books.count(), 1);
}

void tst_QDjangoModel::foreignKey()
{
    QTest::ignoreMessage(QtWarningMsg, "QDjangoMetaModel cannot get foreign model for invalid key 'bad'");
    Book book;
    QVERIFY(!book.foreignKey("bad"));
    QVERIFY(book.foreignKey("author"));
}

void tst_QDjangoModel::foreignKey_null()
{
    QTest::ignoreMessage(QtWarningMsg, "QDjangoMetaModel cannot get foreign model for invalid key 'bad'");
    BookWithNullAuthor book;
    QVERIFY(!book.foreignKey("bad"));
    QVERIFY(!book.foreignKey("author"));
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

void tst_QDjangoModel::filterRelatedReverse()
{
    QDjangoQuerySet<Author> authors;
    QDjangoQuerySet<Author> qs = authors.filter(
                QDjangoWhere("book__title", QDjangoWhere::Equals, "Some book"));
    QVERIFY(!qs.values().isEmpty());
    QCOMPARE(qs.count(), 1);
    QCOMPARE(qs.size(), 1);

    Author *author = qs.at(0);
    QVERIFY(author != 0);
    QCOMPARE(author->name(), QLatin1String("First author"));
    delete author;
}

void tst_QDjangoModel::filterRelatedReverse_null()
{
    QDjangoQuerySet<Author> authors;
    QDjangoQuerySet<Author> qs = authors.filter(
                QDjangoWhere("bookwithnullauthor__title", QDjangoWhere::Equals, "Some book"));
    QVERIFY(!qs.values().isEmpty());
    QCOMPARE(qs.count(), 1);
    QCOMPARE(qs.size(), 1);

    Author *author = qs.at(0);
    QVERIFY(author != 0);
    QCOMPARE(author->name(), QLatin1String("First author"));
    delete author;
}

void tst_QDjangoModel::primaryKey()
{
    Author author;
    QCOMPARE(author.pk(), QVariant());

    author.setPk(1);
    QCOMPARE(author.pk(), QVariant(1));
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

void tst_QDjangoModel::selectRelated_null()
{
    // without eager loading
    QDjangoQuerySet<BookWithNullAuthor> qs;
    BookWithNullAuthor *book = qs.get(QDjangoWhere("title", QDjangoWhere::Equals, "Book with null author"));
    QVERIFY(book != 0);
    QCOMPARE(book->title(), QLatin1String("Book with null author"));
    QVERIFY(!book->author());
    delete book;

    // with eager loading
    book = qs.selectRelated().get(QDjangoWhere("title", QDjangoWhere::Equals, "Book with null author"));
    QVERIFY(book != 0);
    QCOMPARE(book->title(), QLatin1String("Book with null author"));
    QVERIFY(!book->author());
    delete book;
}

void tst_QDjangoModel::toString()
{
    QDjangoQuerySet<Book> qs;
    Book *book = qs.get(QDjangoWhere("title", QDjangoWhere::Equals, "Some book"));
    QVERIFY(book != 0);
    QCOMPARE(book->toString(), QLatin1String("Book(id=1)"));
    delete book;
}

/** Clear database tables after each test.
 */
void tst_QDjangoModel::cleanup()
{
    QVERIFY(QDjango::dropTables());
}

/** Drop database tables after running tests.
 */
void tst_QDjangoModel::cleanupTestCase()
{
}

QTEST_MAIN(tst_QDjangoModel)
#include "tst_qdjangomodel.moc"
