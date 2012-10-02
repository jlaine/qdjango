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

#include <QSqlDriver>

#include "QDjango.h"
#include "QDjangoQuerySet.h"
#include "QDjangoWhere.h"

#include "tst_qdjangocompiler.h"
#include "util.h"

static QString escapeField(const QSqlDatabase &db, const QString &name)
{
    return db.driver()->escapeIdentifier(name, QSqlDriver::FieldName);
}

static QString escapeTable(const QSqlDatabase &db, const QString &name)
{
    return db.driver()->escapeIdentifier(name, QSqlDriver::TableName);
}

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

void tst_QDjangoCompiler::initTestCase()
{
    QVERIFY(initialiseDatabase());
    QDjango::registerModel<Item>();
    QDjango::registerModel<Owner>();
}

void tst_QDjangoCompiler::fieldNames()
{
    QSqlDatabase db = QDjango::database();

    QDjangoCompiler compiler("Owner", db);
    QCOMPARE(compiler.fieldNames(false), QStringList()
        << escapeTable(db, "owner") + "." + escapeField(db, "id")
        << escapeTable(db, "owner") + "." + escapeField(db, "name")
        << escapeTable(db, "owner") + "." + escapeField(db, "item1_id")
        << escapeTable(db, "owner") + "." + escapeField(db, "item2_id"));
    QCOMPARE(compiler.fromSql(), escapeTable(db, "owner"));
}

void tst_QDjangoCompiler::fieldNamesRecursive()
{
    QSqlDatabase db = QDjango::database();

    QDjangoCompiler compiler("Owner", db);
    QCOMPARE(compiler.fieldNames(true), QStringList()
        << escapeTable(db, "owner") + "." + escapeField(db, "id")
        << escapeTable(db, "owner") + "." + escapeField(db, "name")
        << escapeTable(db, "owner") + "." + escapeField(db, "item1_id")
        << escapeTable(db, "owner") + "." + escapeField(db, "item2_id")
        << "T0." + escapeField(db, "id")
        << "T0." + escapeField(db, "name")
        << "T1." + escapeField(db, "id")
        << "T1." + escapeField(db, "name"));
    QCOMPARE(compiler.fromSql(), QString("%1 INNER JOIN %2 T0 ON T0.%3 = %4.%5 INNER JOIN %6 T1 ON T1.%7 = %8.%9").arg(
        escapeTable(db, "owner"),
        escapeTable(db, "item"),
        escapeField(db, "id"),
        escapeTable(db, "owner"),
        escapeField(db, "item1_id"),
        escapeTable(db, "item"),
        escapeField(db, "id"),
        escapeTable(db, "owner"),
        escapeField(db, "item2_id")));
}

void tst_QDjangoCompiler::orderLimit()
{
    QSqlDatabase db = QDjango::database();

    QDjangoCompiler compiler("Owner", db);
    QCOMPARE(compiler.orderLimitSql(QStringList("name"), 0, 0), QString(" ORDER BY %1.%2 ASC").arg(
        escapeTable(db, "owner"),
        escapeField(db, "name")));
    QCOMPARE(compiler.fromSql(), escapeTable(db, "owner"));

    compiler = QDjangoCompiler("Owner", db);
    QCOMPARE(compiler.orderLimitSql(QStringList("-name"), 0, 0), QString(" ORDER BY %1.%2 DESC").arg(
        escapeTable(db, "owner"),
        escapeField(db, "name")));
    QCOMPARE(compiler.fromSql(), escapeTable(db, "owner"));

    compiler = QDjangoCompiler("Owner", db);
    QCOMPARE(compiler.orderLimitSql(QStringList("item1__name"), 0, 0), QString(" ORDER BY T0.%1 ASC").arg(
        escapeField(db, "name")));
    QCOMPARE(compiler.fromSql(), QString("%1 INNER JOIN %2 T0 ON T0.%3 = %4.%5").arg(
        escapeTable(db, "owner"),
        escapeTable(db, "item"),
        escapeField(db, "id"),
        escapeTable(db, "owner"),
        escapeField(db, "item1_id")));
    compiler = QDjangoCompiler("Owner", db);
    QCOMPARE(compiler.orderLimitSql(QStringList() << "item1__name" << "item2__name", 0, 0), QString(" ORDER BY T0.%1 ASC, T1.%2 ASC").arg(
        escapeField(db, "name"),
        escapeField(db, "name")));
    QCOMPARE(compiler.fromSql(), QString("%1 INNER JOIN %2 T0 ON T0.%3 = %4.%5 INNER JOIN %6 T1 ON T1.%7 = %8.%9").arg(
        escapeTable(db, "owner"),
        escapeTable(db, "item"),
        escapeField(db, "id"),
        escapeTable(db, "owner"),
        escapeField(db, "item1_id"),
        escapeTable(db, "item"),
        escapeField(db, "id"),
        escapeTable(db, "owner"),
        escapeField(db, "item2_id")));
}

void tst_QDjangoCompiler::resolve()
{
    QSqlDatabase db = QDjango::database();

    QDjangoCompiler compiler("Owner", db);
    QDjangoWhere where("name", QDjangoWhere::Equals, "foo");
    compiler.resolve(where);
    CHECKWHERE(where, QLatin1String("\"owner\".\"name\" = ?"), QVariantList() << "foo");
    QCOMPARE(compiler.fromSql(), escapeTable(db, "owner"));

    compiler = QDjangoCompiler("Owner", db);
    where = QDjangoWhere("item1__name", QDjangoWhere::Equals, "foo");
    compiler.resolve(where);
    CHECKWHERE(where, QLatin1String("T0.\"name\" = ?"), QVariantList() << "foo");
    QCOMPARE(compiler.fromSql(), QString("%1 INNER JOIN %2 T0 ON T0.%3 = %4.%5").arg(
        escapeTable(db, "owner"),
        escapeTable(db, "item"),
        escapeField(db, "id"),
        escapeTable(db, "owner"),
        escapeField(db, "item1_id")));

    compiler = QDjangoCompiler("Owner", db);
    where = QDjangoWhere("item1__name", QDjangoWhere::Equals, "foo")
         && QDjangoWhere("item2__name", QDjangoWhere::Equals, "bar");
    compiler.resolve(where);
    CHECKWHERE(where, QLatin1String("T0.\"name\" = ? AND T1.\"name\" = ?"), QVariantList() << "foo" << "bar");
    QCOMPARE(compiler.fromSql(), QString("%1 INNER JOIN %2 T0 ON T0.%3 = %4.%5 INNER JOIN %6 T1 ON T1.%7 = %8.%9").arg(
        escapeTable(db, "owner"),
        escapeTable(db, "item"),
        escapeField(db, "id"),
        escapeTable(db, "owner"),
        escapeField(db, "item1_id"),
        escapeTable(db, "item"),
        escapeField(db, "id"),
        escapeTable(db, "owner"),
        escapeField(db, "item2_id")));
}

/** Create database tables before running tests.
 */
void tst_QDjangoModel::initTestCase()
{
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

QTEST_MAIN(tst_QDjangoCompiler)
