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

class Author : public QDjangoModel
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName)

public:
    Author(QObject *parent = 0);

    QString name() const;
    void setName(const QString &name);

private:
    QString m_name;
};

class Book : public QDjangoModel
{
    Q_OBJECT
    Q_PROPERTY(Author* author READ author WRITE setAuthor)
    Q_PROPERTY(QString title READ title WRITE setTitle)

public:
    Book(QObject *parent = 0);

    Author *author() const;
    void setAuthor(Author *author);

    QString title() const;
    void setTitle(const QString &title);

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
    void filterRelated();
    void selectRelated();
    void cleanup();
    void cleanupTestCase();
};

Author::Author(QObject *parent)
    : QDjangoModel(parent)
{
}

QString Author::name() const
{
    return m_name;
}

void Author::setName(const QString &name)
{
    m_name = name;
}

Book::Book(QObject *parent)
    : QDjangoModel(parent)
{
    setForeignKey("author", new Author(this));
}

Author* Book::author() const
{
    return qobject_cast<Author*>(foreignKey("author"));
}

void Book::setAuthor(Author *author)
{
    setForeignKey("author", author);
}

QString Book::title() const
{
    return m_title;
}

void Book::setTitle(const QString &title)
{
    m_title = title;
}

/** Create database tables before running tests.
 */
void tst_QDjangoModel::initTestCase()
{
    QVERIFY(initialiseDatabase());
    QCOMPARE(QDjango::registerModel<Author>().createTable(), true);
    QCOMPARE(QDjango::registerModel<Book>().createTable(), true);
}

/** Perform filtering on foreign keys.
 */
void tst_QDjangoModel::filterRelated()
{
    // load fixtures
    {
        Author author;
        author.setName("first");
        QCOMPARE(author.save(), true);

        Book book;
        book.setAuthor(&author);
        book.setTitle("Some book");
        QCOMPARE(book.save(), true);
    }

    // perform filtering
    QDjangoQuerySet<Book> books;

    QDjangoQuerySet<Book> qs = books.filter(
        QDjangoWhere("author__name", QDjangoWhere::Equals, "first"));
    CHECKWHERE(qs.where(), QLatin1String("T0.\"name\" = ?"), QVariantList() << "first");
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
    // load fixtures
    {
        Author author;
        author.setName("first");
        QCOMPARE(author.save(), true);

        Book book;
        book.setAuthor(&author);
        book.setTitle("Some book");
        QCOMPARE(book.save(), true);
    }

    // without eager loading
    QDjangoQuerySet<Book> qs;
    Book *book = qs.get(QDjangoWhere("title", QDjangoWhere::Equals, "Some book"));
    QVERIFY(book != 0);
    QCOMPARE(book->title(), QLatin1String("Some book"));
    QCOMPARE(book->author()->name(), QLatin1String("first"));
    delete book;

    // with eager loading
    book = qs.selectRelated().get(QDjangoWhere("title", QDjangoWhere::Equals, "Some book"));
    QVERIFY(book != 0);
    QCOMPARE(book->title(), QLatin1String("Some book"));
    QCOMPARE(book->author()->name(), QLatin1String("first"));
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
