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

#include "QDjango.h"
#include "QDjangoModel.h"
#include "QDjangoQuerySet.h"
#include "QDjangoWhere.h"

#include "util.h"

class Publication : public QDjangoModel
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle)

public:
    Publication(QObject *parent = 0) : QDjangoModel(parent) {}

    QString title() const { return m_title; }
    void setTitle(const QString &title) { m_title = title; }

private:
    QString m_title;
};

class Article : public QDjangoModel
{
    Q_OBJECT
    Q_PROPERTY(QString headline READ headline WRITE setHeadline)
    Q_PROPERTY(QList<Publication*> publications)

public:
    Article(QObject *parent = 0) : QDjangoModel(parent) {}

    QString headline() const { return m_headline; }
    void setHeadline(const QString &headline) { m_headline = headline; }

private:
    QString m_headline;

};

class Article_Publication : public QDjangoModel
{
    Q_OBJECT
    Q_PROPERTY(Article* article READ article WRITE setArticle)
    Q_PROPERTY(Publication* publication READ publication WRITE setPublication)
    Q_CLASSINFO("article", "on_delete=cascade on_update=cascade")
    Q_CLASSINFO("publication", "on_delete=cascade on_update=cascade")

public:
    Article_Publication(QObject *parent = 0)
        : QDjangoModel(parent)
    {
        setForeignKey("article", new Article(this));
        setForeignKey("publication", new Publication(this));
    }

    Article *article() const
    {
        return qobject_cast<Article*>(foreignKey("article"));
    }

    void setArticle(Article *article)
    {
        setForeignKey("article", article);
    }

    Publication *publication() const
    {
        return qobject_cast<Publication*>(foreignKey("publication"));
    }

    void setPublication(Publication *publication)
    {
        setForeignKey("publication", publication);
    }
};

/** Test QDjangoModel class.
 */
class tst_ManyToMany : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void init();
    void cleanup();
    void cleanupTestCase();

    void filterManyToManyRelated();

};

/** Create database tables before running tests.
 */
void tst_ManyToMany::initTestCase()
{
    QVERIFY(initialiseDatabase());
    QCOMPARE(QDjango::registerModel<Article>().createTable(), true);
    QCOMPARE(QDjango::registerModel<Publication>().createTable(), true);
    QCOMPARE(QDjango::registerModel<Article_Publication>().createTable(), true);
}

/** Load fixtures.
 */
void tst_ManyToMany::init()
{
    Article article1;
    article1.setHeadline("Zombie Attack!");
    QCOMPARE(article1.save(), true);

    Article article2;
    article2.setHeadline("Oprah, The Devil");
    QCOMPARE(article2.save(), true);

    Article article3;
    article3.setHeadline("Huzzah");
    QCOMPARE(article3.save(), true);

    Publication publication1;
    publication1.setTitle("first publication");
    QCOMPARE(publication1.save(), true);

    Publication publication2;
    publication2.setTitle("second publication");
    QCOMPARE(publication2.save(), true);

    Publication publication3;
    publication3.setTitle("third publication");
    QCOMPARE(publication3.save(), true);

    // Article1 -> (Publication1, Publication2)
    {
        Article_Publication m2mRelations;
        m2mRelations.setArticle(&article1);
        m2mRelations.setPublication(&publication1);
        QCOMPARE(m2mRelations.save(), true);
    }
    {
        Article_Publication m2mRelations;
        m2mRelations.setArticle(&article1);
        m2mRelations.setPublication(&publication2);
        QCOMPARE(m2mRelations.save(), true);
    }

    // Article2 -> (Publication1, Publication3)
    {
        Article_Publication m2mRelations;
        m2mRelations.setArticle(&article2);
        m2mRelations.setPublication(&publication1);
        QCOMPARE(m2mRelations.save(), true);
    }
    {
        Article_Publication m2mRelations;
        m2mRelations.setArticle(&article2);
        m2mRelations.setPublication(&publication3);
        QCOMPARE(m2mRelations.save(), true);
    }

    // Article3 -> (Publication3)
    {
        Article_Publication m2mRelations;
        m2mRelations.setArticle(&article3);
        m2mRelations.setPublication(&publication3);
        QCOMPARE(m2mRelations.save(), true);
    }
}

/** Clear database tables after each test.
 */
void tst_ManyToMany::cleanup()
{
    QCOMPARE(QDjangoQuerySet<Article_Publication>().remove(), true);
    QCOMPARE(QDjangoQuerySet<Article>().remove(), true);
    QCOMPARE(QDjangoQuerySet<Publication>().remove(), true);
}

/** Drop database tables after running tests.
 */
void tst_ManyToMany::cleanupTestCase()
{
    QCOMPARE(QDjango::registerModel<Article_Publication>().dropTable(), true);
    QCOMPARE(QDjango::registerModel<Article>().dropTable(), true);
    QCOMPARE(QDjango::registerModel<Publication>().dropTable(), true);
}

void tst_ManyToMany::filterManyToManyRelated()
{
    QDjangoQuerySet<Article> articles;
    QList<QVariantMap> entries =
        articles.filter(QDjangoWhere("publication__title", QDjangoWhere::Contains, "first")).values();
    QCOMPARE(entries.size(), 2);

    int count =
         articles.filter(QDjangoWhere("publication__title", QDjangoWhere::Contains, "first")).count();
    QCOMPARE(count, 2);

    count =
        articles.filter(QDjangoWhere("publication__title", QDjangoWhere::Contains, "second")).count();
    QCOMPARE(count, 1);

    count =
        articles.filter(QDjangoWhere("publication__title", QDjangoWhere::Contains, "third")).count();
    QCOMPARE(count, 2);
}

QTEST_MAIN(tst_ManyToMany)
#include "tst_manytomany.moc"
