/*
 * Copyright (C) 2010-2013 Jeremy Lainé
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

#include "util.h"

static QString escapeField(const QSqlDatabase &db, const QString &name)
{
    return db.driver()->escapeIdentifier(name, QSqlDriver::FieldName);
}

static QString escapeTable(const QSqlDatabase &db, const QString &name)
{
    return db.driver()->escapeIdentifier(name, QSqlDriver::TableName);
}

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

class OwnerWithNullableItem : public QDjangoModel
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(Item* item1 READ item1 WRITE setItem1)
    Q_PROPERTY(Item* item2 READ item2 WRITE setItem2)
    Q_CLASSINFO("item2", "null=true")

public:
    OwnerWithNullableItem(QObject *parent = 0);

    QString name() const;
    void setName(const QString &name);

    Item *item1() const;
    void setItem1(Item *item1);

    Item *item2() const;
    void setItem2(Item *item2);

private:
    QString m_name;
};

class tst_QDjangoCompiler : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void fieldNames();
    void fieldNamesRecursive();
    void fieldNamesNullable();
    void orderLimit();
    void resolve();
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

OwnerWithNullableItem::OwnerWithNullableItem(QObject *parent)
    : QDjangoModel(parent)
{
    setForeignKey("item1", new Item(this));
    setForeignKey("item2", new Item(this));
}

QString OwnerWithNullableItem::name() const
{
    return m_name;
}

void OwnerWithNullableItem::setName(const QString &name)
{
    m_name = name;
}

Item* OwnerWithNullableItem::item1() const
{
    return qobject_cast<Item*>(foreignKey("item1"));
}

void OwnerWithNullableItem::setItem1(Item *item1)
{
    setForeignKey("item1", item1);
}

Item* OwnerWithNullableItem::item2() const
{
    return qobject_cast<Item*>(foreignKey("item2"));
}

void OwnerWithNullableItem::setItem2(Item *item2)
{
    setForeignKey("item2", item2);
}

void tst_QDjangoCompiler::initTestCase()
{
    QVERIFY(initialiseDatabase());
    QDjango::registerModel<Item>();
    QDjango::registerModel<Owner>();
    QDjango::registerModel<OwnerWithNullableItem>();
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

void tst_QDjangoCompiler::fieldNamesNullable()
{
    QSqlDatabase db = QDjango::database();

    QDjangoCompiler compiler("OwnerWithNullableItem", db);
    QCOMPARE(compiler.fieldNames(true), QStringList()
        << escapeTable(db, "ownerwithnullableitem") + "." + escapeField(db, "id")
        << escapeTable(db, "ownerwithnullableitem") + "." + escapeField(db, "name")
        << escapeTable(db, "ownerwithnullableitem") + "." + escapeField(db, "item1_id")
        << escapeTable(db, "ownerwithnullableitem") + "." + escapeField(db, "item2_id")
        << "T0." + escapeField(db, "id")
        << "T0." + escapeField(db, "name")
        << "T1." + escapeField(db, "id")
        << "T1." + escapeField(db, "name"));
    QCOMPARE(compiler.fromSql(), QString("%1 INNER JOIN %2 T0 ON T0.%3 = %4.%5 LEFT OUTER JOIN %6 T1 ON T1.%7 = %8.%9").arg(
        escapeTable(db, "ownerwithnullableitem"),
        escapeTable(db, "item"),
        escapeField(db, "id"),
        escapeTable(db, "ownerwithnullableitem"),
        escapeField(db, "item1_id"),
        escapeTable(db, "item"),
        escapeField(db, "id"),
        escapeTable(db, "ownerwithnullableitem"),
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

QTEST_MAIN(tst_QDjangoCompiler)
#include "tst_qdjangocompiler.moc"
