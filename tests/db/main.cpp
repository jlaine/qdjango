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

#include <cstdlib>

#include <QCoreApplication>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QVariant>
#include <QtTest>

#include "QDjango.h"
#include "QDjangoQuerySet.h"
#include "QDjangoWhere.h"

#include "main.h"
#include "auth-models.h"
#include "auth-tests.h"
#include "fields.h"
#include "foreignkey.h"
#include "shares-models.h"
#include "shares-tests.h"
#include "util.h"

static QString escapeField(const QSqlDatabase &db, const QString &name)
{
    return db.driver()->escapeIdentifier(name, QSqlDriver::FieldName);
}

static QString escapeTable(const QSqlDatabase &db, const QString &name)
{
    return db.driver()->escapeIdentifier(name, QSqlDriver::TableName);
}

Object::Object(QObject *parent)
    : QObject(parent)
    , m_bar(0)
    , m_wiz(0)
    , m_zoo(0)
    , m_zzz(0)
{
}

QString Object::foo() const
{
    return m_foo;
}

void Object::setFoo(const QString &foo)
{
    m_foo = foo;
}

int Object::bar() const
{
    return m_bar;
}

void Object::setBar(int bar)
{
    m_bar = bar;
}

int Object::wiz() const
{
    return m_wiz;
}

void Object::setWiz(int wiz)
{
    m_wiz = wiz;
}

int Object::zoo() const
{
    return m_zoo;
}

void Object::setZoo(int zoo)
{
    m_zoo = zoo;
}

int Object::zzz() const
{
    return m_zzz;
}

void Object::setZzz(int zzz)
{
    m_zzz = zzz;
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

void tst_QDjangoMetaModel::initTestCase()
{
    metaModel = QDjango::registerModel<Object>();
    QCOMPARE(metaModel.createTable(), true);
}

void tst_QDjangoMetaModel::localField_data()
{
    QTest::addColumn<QString>("lookup");
    QTest::addColumn<bool>("isValid");
    QTest::addColumn<QString>("name");
    QTest::addColumn<QString>("column");

    QTest::newRow("pk") << "pk" << true << "id" << "id";
    QTest::newRow("id") << "id" << true << "id" << "id";
    QTest::newRow("foo") << "foo" << true << "foo" << "foo";
    QTest::newRow("bar") << "bar" << true << "bar" << "bar";
    QTest::newRow("zoo") << "zoo" << true << "zoo" << "zoo";
    QTest::newRow("zzz") << "zzz" << true << "zzz" << "zzz_column";
    QTest::newRow("unknown") << "unknown" << false << "" << "";
}

void tst_QDjangoMetaModel::localField()
{
    QFETCH(QString, lookup);
    QFETCH(bool, isValid);
    QFETCH(QString, name);
    QFETCH(QString, column);

    QDjangoMetaField field = metaModel.localField(lookup.toLatin1());
    QCOMPARE(field.isValid(), isValid);
    QCOMPARE(field.name(), name);
    QCOMPARE(field.column(), column);
}

void tst_QDjangoMetaModel::options()
{
    const QList<QDjangoMetaField> localFields = metaModel.localFields();
    QCOMPARE(metaModel.table(), QLatin1String("foo_table"));
    QCOMPARE(metaModel.primaryKey(), QByteArray("id"));
    QCOMPARE(localFields.size(), 5);
    QCOMPARE(localFields[0].name(), QLatin1String("id"));
    QCOMPARE(localFields[0].column(), QLatin1String("id"));
#if 0
    QCOMPARE(localFields[0].autoIncrement, true);
    QCOMPARE(localFields[0].maxLength, 0);
    QCOMPARE(localFields[0].index, false);
    QCOMPARE(localFields[0].unique, true);
#endif
    QCOMPARE(localFields[1].name(), QLatin1String("foo"));
    QCOMPARE(localFields[1].column(), QLatin1String("foo"));
#if 0
    QCOMPARE(localFields[1].autoIncrement, false);
    QCOMPARE(localFields[1].index, false);
    QCOMPARE(localFields[1].maxLength, 255);
    QCOMPARE(localFields[1].unique, false);
#endif
    QCOMPARE(localFields[2].name(), QLatin1String("bar"));
    QCOMPARE(localFields[2].column(), QLatin1String("bar"));
#if 0
    QCOMPARE(localFields[2].autoIncrement, false);
    QCOMPARE(localFields[2].index, true);
    QCOMPARE(localFields[2].maxLength, 0);
    QCOMPARE(localFields[2].unique, false);
#endif
    QCOMPARE(localFields[3].name(), QLatin1String("zoo"));
    QCOMPARE(localFields[3].column(), QLatin1String("zoo"));
#if 0
    QCOMPARE(localFields[3].autoIncrement, false);
    QCOMPARE(localFields[3].index, false);
    QCOMPARE(localFields[3].maxLength, 0);
    QCOMPARE(localFields[3].unique, true);
#endif
    QCOMPARE(localFields[4].name(), QLatin1String("zzz"));
    QCOMPARE(localFields[4].column(), QLatin1String("zzz_column"));
}

void tst_QDjangoMetaModel::save()
{
    Object obj;
    obj.setFoo("some string");
    obj.setBar(1234);
    QCOMPARE(metaModel.save(&obj), true);
    QCOMPARE(obj.property("id"), QVariant(1));

    // save again
    QCOMPARE(metaModel.save(&obj), true);
    QCOMPARE(obj.property("id"), QVariant(1));
}

void tst_QDjangoMetaModel::cleanupTestCase()
{
    metaModel.dropTable();
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
        Item *item1 = new Item;
        item1->setName("first");
        QCOMPARE(item1->save(), true);

        Item *item2 = new Item;
        item2->setName("second");
        QCOMPARE(item2->save(), true);

        Owner owner;
        owner.setName("owner");
        owner.setItem1(item1);
        owner.setItem2(item2);
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
        Item *item1 = new Item;
        item1->setName("first");
        QCOMPARE(item1->save(), true);

        Item *item2 = new Item;
        item2->setName("second");
        QCOMPARE(item2->save(), true);

        Owner owner;
        owner.setName("owner");
        owner.setItem1(item1);
        owner.setItem2(item2);
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

void tst_QDjangoQuerySetPrivate::initTestCase()
{
    metaModel = QDjango::registerModel<Object>();
    QCOMPARE(metaModel.createTable(), true);
}

void tst_QDjangoQuerySetPrivate::deleteQuery()
{
    QDjangoQuerySetPrivate qs("Object");
    qs.addFilter(QDjangoWhere("pk", QDjangoWhere::Equals, 1));
    QDjangoQuery query = qs.deleteQuery();

    QCOMPARE(normalizeSql(QDjango::database(), query.lastQuery()), QLatin1String("DELETE FROM \"foo_table\" WHERE \"foo_table\".\"id\" = ?"));
    QCOMPARE(query.boundValues().size(), 1);
    QCOMPARE(query.boundValue(0), QVariant(1));
}

void tst_QDjangoQuerySetPrivate::insertQuery()
{
    QVariantMap data;
    data.insert("foo", 2);

    QDjangoQuerySetPrivate qs("Object");
    QDjangoQuery query = qs.insertQuery(data);

    QCOMPARE(normalizeSql(QDjango::database(), query.lastQuery()), QLatin1String("INSERT INTO \"foo_table\" (\"foo\") VALUES(?)"));
    QCOMPARE(query.boundValues().size(), 1);
    QCOMPARE(query.boundValue(0), QVariant(2));
}

void tst_QDjangoQuerySetPrivate::updateQuery()
{
    QVariantMap data;
    data.insert("foo", 2);

    {
        QDjangoQuerySetPrivate qs("Object");
        qs.addFilter(QDjangoWhere("pk", QDjangoWhere::Equals, 1));
        QDjangoQuery query = qs.updateQuery(data);

        QCOMPARE(normalizeSql(QDjango::database(), query.lastQuery()), QLatin1String("UPDATE \"foo_table\" SET \"foo\" = ? WHERE \"foo_table\".\"id\" = ?"));
        QCOMPARE(query.boundValues().size(), 2);
        QCOMPARE(query.boundValue(0), QVariant(2));
        QCOMPARE(query.boundValue(1), QVariant(1));
    }

    {
        QDjangoQuerySetPrivate qs("Object");
        qs.addFilter(QDjangoWhere("zzz", QDjangoWhere::Equals, 3));
        QDjangoQuery query = qs.updateQuery(data);

        QCOMPARE(normalizeSql(QDjango::database(), query.lastQuery()), QLatin1String("UPDATE \"foo_table\" SET \"foo\" = ? WHERE \"foo_table\".\"zzz_column\" = ?"));
        QCOMPARE(query.boundValue(0), QVariant(2));
        QCOMPARE(query.boundValue(1), QVariant(3));
    }
}

void tst_QDjangoQuerySetPrivate::cleanupTestCase()
{
    metaModel.dropTable();
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    initialiseDatabase();

    // declare models
    QDjango::registerModel<User>();
    QDjango::registerModel<Group>();
    QDjango::registerModel<Message>();
    QDjango::registerModel<UserGroups>();

    // run tests
    int errors = 0;

    tst_QDjangoCompiler testCompiler;
    errors += QTest::qExec(&testCompiler);

    tst_QDjangoMetaModel testMetaModel;
    errors += QTest::qExec(&testMetaModel);

    tst_QDjangoQuerySetPrivate testQuerySetPrivate;
    errors += QTest::qExec(&testQuerySetPrivate);

    tst_QDjangoModel testModel;
    errors += QTest::qExec(&testModel);

    // fields
    tst_Fields testFields;
    errors += QTest::qExec(&testFields);

    tst_FkConstraint testFkConstraint;
    errors += QTest::qExec(&testFkConstraint);

    // models
    TestUser testUser;
    errors += QTest::qExec(&testUser);

    TestRelated testRelated;
    errors += QTest::qExec(&testRelated);

    TestShares testShares;
    errors += QTest::qExec(&testShares);

    if (errors) {
        qWarning() << "Total failed tests:" << errors;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
};

