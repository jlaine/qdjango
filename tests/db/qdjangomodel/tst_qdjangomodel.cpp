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

class Item : public QDjangoModel
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName)

public:
    Item(QObject *parent = 0);

    QString name() const;
    void setName(const QString &name);

private:
    QString m_name;
};

class Owner : public QDjangoModel
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(Item* item1 READ item1 WRITE setItem1)
    Q_PROPERTY(Item* item2 READ item2 WRITE setItem2)

public:
    Owner(QObject *parent = 0);

    QString name() const;
    void setName(const QString &name);

    Item *item1() const;
    void setItem1(Item *item1);

    Item *item2() const;
    void setItem2(Item *item2);

private:
    QString m_name;
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

Item::Item(QObject *parent)
    : QDjangoModel(parent)
{
}

QString Item::name() const
{
    return m_name;
}

void Item::setName(const QString &name)
{
    m_name = name;
}

Owner::Owner(QObject *parent)
    : QDjangoModel(parent)
{
    setForeignKey("item1", new Item(this));
    setForeignKey("item2", new Item(this));
}

QString Owner::name() const
{
    return m_name;
}

void Owner::setName(const QString &name)
{
    m_name = name;
}

Item* Owner::item1() const
{
    return qobject_cast<Item*>(foreignKey("item1"));
}

void Owner::setItem1(Item *item1)
{
    setForeignKey("item1", item1);
}

Item* Owner::item2() const
{
    return qobject_cast<Item*>(foreignKey("item2"));
}

void Owner::setItem2(Item *item2)
{
    setForeignKey("item2", item2);
}

/** Create database tables before running tests.
 */
void tst_QDjangoModel::initTestCase()
{
    QVERIFY(initialiseDatabase());
    QCOMPARE(QDjango::registerModel<Item>().createTable(), true);
    QCOMPARE(QDjango::registerModel<Owner>().createTable(), true);
}

/** Perform filtering on foreign keys.
 */
void tst_QDjangoModel::filterRelated()
{
    // load fixtures
    {
        Item item1;
        item1.setName("first");
        QCOMPARE(item1.save(), true);

        Item item2;
        item2.setName("second");
        QCOMPARE(item2.save(), true);

        Owner owner;
        owner.setName("owner");
        owner.setItem1(&item1);
        owner.setItem2(&item2);
        QCOMPARE(owner.save(), true);
    }

    // perform filtering
    QDjangoQuerySet<Owner> owners;

    QDjangoQuerySet<Owner> qs = owners.filter(
        QDjangoWhere("item1__name", QDjangoWhere::Equals, "first"));
    CHECKWHERE(qs.where(), QLatin1String("T0.\"name\" = ?"), QVariantList() << "first");
    QCOMPARE(qs.count(), 1);
    QCOMPARE(qs.size(), 1);

    Owner *owner = qs.at(0);
    QVERIFY(owner != 0);
    QCOMPARE(owner->name(), QLatin1String("owner"));
    delete owner;
}

/** Test eager loading of foreign keys.
 */
void tst_QDjangoModel::selectRelated()
{
    // load fixtures
    {
        Item item1;
        item1.setName("first");
        QCOMPARE(item1.save(), true);

        Item item2;
        item2.setName("second");
        QCOMPARE(item2.save(), true);

        Owner owner;
        owner.setName("owner");
        owner.setItem1(&item1);
        owner.setItem2(&item2);
        QCOMPARE(owner.save(), true);
    }

    // without eager loading
    QDjangoQuerySet<Owner> qs;
    Owner *owner = qs.get(QDjangoWhere("name", QDjangoWhere::Equals, "owner"));
    QVERIFY(owner != 0);
    QCOMPARE(owner->item1()->name(), QLatin1String("first"));
    QCOMPARE(owner->item2()->name(), QLatin1String("second"));
    delete owner;

    // with eager loading
    owner = qs.selectRelated().get(QDjangoWhere("name", QDjangoWhere::Equals, "owner"));
    QVERIFY(owner != 0);
    QCOMPARE(owner->item1()->name(), QLatin1String("first"));
    QCOMPARE(owner->item2()->name(), QLatin1String("second"));
    delete owner;
}

/** Clear database tables after each test.
 */
void tst_QDjangoModel::cleanup()
{
    QCOMPARE(QDjangoQuerySet<Owner>().remove(), true);
    QCOMPARE(QDjangoQuerySet<Item>().remove(), true);
}

/** Drop database tables after running tests.
 */
void tst_QDjangoModel::cleanupTestCase()
{
    QCOMPARE(QDjango::registerModel<Owner>().dropTable(), true);
    QCOMPARE(QDjango::registerModel<Item>().dropTable(), true);
}

QTEST_MAIN(tst_QDjangoModel)
#include "tst_qdjangomodel.moc"
