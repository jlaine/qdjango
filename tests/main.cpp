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
#include "shares-models.h"
#include "shares-tests.h"
#include "http.h"
#include "script.h"

static QString escapeField(const QSqlDatabase &db, const QString &name)
{
    return db.driver()->escapeIdentifier(name, QSqlDriver::FieldName);
}

static QString escapeTable(const QSqlDatabase &db, const QString &name)
{
    return db.driver()->escapeIdentifier(name, QSqlDriver::TableName);
}

Object::Object(QObject *parent)
    : QObject(parent),
    m_bar(0),
    m_wiz(0),
    m_zoo(0)
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

void tst_QDjangoMetaModel::options()
{
    const QList<QDjangoMetaField> localFields = metaModel.localFields();
    QCOMPARE(metaModel.table(), QLatin1String("foo_table"));
    QCOMPARE(metaModel.primaryKey(), QByteArray("id"));
    QCOMPARE(localFields.size(), 4);
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

/** Test empty where clause.
 */
void tst_QDjangoWhere::emptyWhere()
{
    // construct empty where clause
    QDjangoWhere testQuery;
    QCOMPARE(testQuery.isAll(), true);
    QCOMPARE(testQuery.isNone(), false);
    CHECKWHERE(testQuery, QString(), QVariantList());

    // negate the where clause
    testQuery = !QDjangoWhere();
    QCOMPARE(testQuery.isAll(), false);
    QCOMPARE(testQuery.isNone(), true);
    CHECKWHERE(testQuery, QLatin1String("1 != 0"), QVariantList());
}

/** Test "=" comparison.
 */
void tst_QDjangoWhere::equalsWhere()
{
    QDjangoWhere testQuery;

    // construct an "equals" where clause
    testQuery = QDjangoWhere("id", QDjangoWhere::Equals, 1);
    CHECKWHERE(testQuery, QLatin1String("id = ?"), QVariantList() << 1);

    // negate the where clause
    testQuery = !QDjangoWhere("id", QDjangoWhere::Equals, 1);
    CHECKWHERE(testQuery, QLatin1String("id != ?"), QVariantList() << 1);
}

/** Test "!=" comparison.
 */
void tst_QDjangoWhere::notEqualsWhere()
{
    QDjangoWhere testQuery;

    // construct a "not equals" where clause
    testQuery = QDjangoWhere("id", QDjangoWhere::NotEquals, 1);
    CHECKWHERE(testQuery, QLatin1String("id != ?"), QVariantList() << 1);

    // negate the where clause
    testQuery = !QDjangoWhere("id", QDjangoWhere::NotEquals, 1);
    CHECKWHERE(testQuery, QLatin1String("id = ?"), QVariantList() << 1);
}

/** Test ">" comparison.
 */
void tst_QDjangoWhere::greaterThan()
{
    QDjangoWhere testQuery;

    testQuery = QDjangoWhere("id", QDjangoWhere::GreaterThan, 1);
    CHECKWHERE(testQuery, QLatin1String("id > ?"), QVariantList() << 1);

    testQuery = !QDjangoWhere("id", QDjangoWhere::GreaterThan, 1);
    CHECKWHERE(testQuery, QLatin1String("id <= ?"), QVariantList() << 1);
}

/** Test ">=" comparison.
 */
void tst_QDjangoWhere::greaterOrEquals()
{
    QDjangoWhere testQuery;

    testQuery = QDjangoWhere("id", QDjangoWhere::GreaterOrEquals, 1);
    CHECKWHERE(testQuery, QLatin1String("id >= ?"), QVariantList() << 1);

    testQuery = !QDjangoWhere("id", QDjangoWhere::GreaterOrEquals, 1);
    CHECKWHERE(testQuery, QLatin1String("id < ?"), QVariantList() << 1);
}

/** Test "<" comparison.
 */
void tst_QDjangoWhere::lessThan()
{
    QDjangoWhere testQuery;

    testQuery = QDjangoWhere("id", QDjangoWhere::LessThan, 1);
    CHECKWHERE(testQuery, QLatin1String("id < ?"), QVariantList() << 1);

    testQuery = !QDjangoWhere("id", QDjangoWhere::LessThan, 1);
    CHECKWHERE(testQuery, QLatin1String("id >= ?"), QVariantList() << 1);
}

/** Test "<=" comparison.
 */
void tst_QDjangoWhere::lessOrEquals()
{
    QDjangoWhere testQuery;

    testQuery = QDjangoWhere("id", QDjangoWhere::LessOrEquals, 1);
    CHECKWHERE(testQuery, QLatin1String("id <= ?"), QVariantList() << 1);

    testQuery = !QDjangoWhere("id", QDjangoWhere::LessOrEquals, 1);
    CHECKWHERE(testQuery, QLatin1String("id > ?"), QVariantList() << 1);
}

/** Test "in" comparison.
 */
void tst_QDjangoWhere::isIn()
{
    QDjangoWhere testQuery = QDjangoWhere("id", QDjangoWhere::IsIn, QVariantList() << 1 << 2);
    CHECKWHERE(testQuery, QLatin1String("id IN (?, ?)"), QVariantList() << 1 << 2);

    testQuery = !QDjangoWhere("id", QDjangoWhere::IsIn, QVariantList() << 1 << 2);
    CHECKWHERE(testQuery, QLatin1String("id NOT IN (?, ?)"), QVariantList() << 1 << 2);
}

/** Test "isnull" comparison.
 */
void tst_QDjangoWhere::isNull()
{
    QDjangoWhere testQuery = QDjangoWhere("id", QDjangoWhere::IsNull, true);
    CHECKWHERE(testQuery, QLatin1String("id IS NULL"), QVariantList());

    testQuery = QDjangoWhere("id", QDjangoWhere::IsNull, false);
    CHECKWHERE(testQuery, QLatin1String("id IS NOT NULL"), QVariantList());

    testQuery = !QDjangoWhere("id", QDjangoWhere::IsNull, true);
    CHECKWHERE(testQuery, QLatin1String("id IS NOT NULL"), QVariantList());
}

/** Test "startswith" comparison.
 */
void tst_QDjangoWhere::startsWith()
{
    QDjangoWhere testQuery = QDjangoWhere("name", QDjangoWhere::StartsWith, "abc");
    CHECKWHERE(testQuery, QLatin1String("name LIKE ?"), QVariantList() << "abc%");

    testQuery = !QDjangoWhere("name", QDjangoWhere::StartsWith, "abc");
    CHECKWHERE(testQuery, QLatin1String("name NOT LIKE ?"), QVariantList() << "abc%");
}

/** Test "endswith" comparison.
 */
void tst_QDjangoWhere::endsWith()
{
    QDjangoWhere testQuery = QDjangoWhere("name", QDjangoWhere::EndsWith, "abc");
    CHECKWHERE(testQuery, QLatin1String("name LIKE ?"), QVariantList() << "%abc");

    testQuery = !QDjangoWhere("name", QDjangoWhere::EndsWith, "abc");
    CHECKWHERE(testQuery, QLatin1String("name NOT LIKE ?"), QVariantList() << "%abc");
}

/** Test "contains" comparison.
 */
void tst_QDjangoWhere::contains()
{
    QDjangoWhere testQuery = QDjangoWhere("name", QDjangoWhere::Contains, "abc");
    CHECKWHERE(testQuery, QLatin1String("name LIKE ?"), QVariantList() << "%abc%");

    testQuery = !QDjangoWhere("name", QDjangoWhere::Contains, "abc");
    CHECKWHERE(testQuery, QLatin1String("name NOT LIKE ?"), QVariantList() << "%abc%");
}

/** Test compound where clause, using the AND operator.
 */
void tst_QDjangoWhere::andWhere()
{
    QDjangoWhere testQuery;

    const QDjangoWhere queryId("id", QDjangoWhere::Equals, 1);
    const QDjangoWhere queryUsername("username", QDjangoWhere::Equals, "foo");

    testQuery = queryId && queryUsername;
    CHECKWHERE(testQuery, QLatin1String("id = ? AND username = ?"), QVariantList() << 1 << "foo");

    // and with "all" queryset
    testQuery = QDjangoWhere() && queryId;
    CHECKWHERE(testQuery, QLatin1String("id = ?"), QVariantList() << 1);

    testQuery = queryId && QDjangoWhere();
    CHECKWHERE(testQuery, QLatin1String("id = ?"), QVariantList() << 1);

    // and with "none" queryset
    testQuery = !QDjangoWhere() && queryId;
    QCOMPARE(testQuery.isNone(), true);
    CHECKWHERE(testQuery, QLatin1String("1 != 0"), QVariantList());

    testQuery = queryId && !QDjangoWhere();
    QCOMPARE(testQuery.isNone(), true);
    CHECKWHERE(testQuery, QLatin1String("1 != 0"), QVariantList());

    // negation
    testQuery = !(queryId && queryUsername);
    CHECKWHERE(testQuery, QLatin1String("NOT (id = ? AND username = ?)"), QVariantList() << 1 << "foo");
}

/** Test compound where clause, using the OR operator.
 */
void tst_QDjangoWhere::orWhere()
{
    QDjangoWhere testQuery;

    const QDjangoWhere queryId("id", QDjangoWhere::Equals, 1);
    const QDjangoWhere queryUsername("username", QDjangoWhere::Equals, "foo");

    testQuery = queryId || queryUsername;
    CHECKWHERE(testQuery, QLatin1String("id = ? OR username = ?"), QVariantList() << 1 << "foo");

    // or with "all" queryset
    testQuery = QDjangoWhere() || queryId;
    QCOMPARE(testQuery.isAll(), true);
    CHECKWHERE(testQuery, QString(), QVariantList());

    testQuery = queryId || QDjangoWhere();
    QCOMPARE(testQuery.isAll(), true);
    CHECKWHERE(testQuery, QString(), QVariantList());

    // or with "none" queryset
    testQuery = !QDjangoWhere() || queryId;
    CHECKWHERE(testQuery, QLatin1String("id = ?"), QVariantList() << 1);

    testQuery = queryId || !QDjangoWhere();
    CHECKWHERE(testQuery, QLatin1String("id = ?"), QVariantList() << 1);

    // negation
    testQuery = !(queryId || queryUsername);
    CHECKWHERE(testQuery, QLatin1String("NOT (id = ? OR username = ?)"), QVariantList() << 1 << "foo");
}

/** Test compound where clause, using both the AND and the OR operators.
 */
void tst_QDjangoWhere::complexWhere()
{
    QDjangoWhere testQuery;

    const QDjangoWhere queryId("id", QDjangoWhere::Equals, 1);
    const QDjangoWhere queryUsername("username", QDjangoWhere::Equals, "foouser");
    const QDjangoWhere queryPassword("password", QDjangoWhere::Equals, "foopass");

    testQuery = (queryId || queryUsername) && queryPassword;
    CHECKWHERE(testQuery, QLatin1String("(id = ? OR username = ?) AND password = ?"), QVariantList() << 1 << "foouser" << "foopass");

    testQuery = queryId || (queryUsername && queryPassword);
    CHECKWHERE(testQuery, QLatin1String("id = ? OR (username = ? AND password = ?)"), QVariantList() << 1 << "foouser" << "foopass");
}

/** Display program usage.
 */
static void usage()
{
    fprintf(stderr, "Usage: qdjango-test [-d <driver>] [-h <host>] [-n <database>] [-u <user>] [-p <password>]\n");
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // initialise options
    QString databaseDriver = "QSQLITE";
    QString databaseName = ":memory:";
    QString databaseUser;
    QString databasePassword;
    QString databaseHost;
    int count = 1;

    // parse command line arguments
    if (!(argc % 2))
    {
        usage();
        return EXIT_FAILURE;
    }
    for (int i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "-c") && i < argc - 1)
            count = QString::fromLocal8Bit(argv[++i]).toInt();
        else if (!strcmp(argv[i], "-d") && i < argc - 1)
            databaseDriver = QString::fromLocal8Bit(argv[++i]);
        else if (!strcmp(argv[i], "-h") && i < argc - 1)
            databaseHost = QString::fromLocal8Bit(argv[++i]);
        else if (!strcmp(argv[i], "-n") && i < argc - 1)
            databaseName = QString::fromLocal8Bit(argv[++i]);
        else if (!strcmp(argv[i], "-p") && i < argc - 1)
            databasePassword = QString::fromLocal8Bit(argv[++i]);
        else if (!strcmp(argv[i], "-u") && i < argc - 1)
            databaseUser = QString::fromLocal8Bit(argv[++i]);
        else
        {
            usage();
            return EXIT_FAILURE;
        }
    }

    // enabled SQL debugging
    QDjango::setDebugEnabled(true);

    // open database
    QSqlDatabase db = QSqlDatabase::addDatabase(databaseDriver);
    db.setDatabaseName(databaseName);
    db.setUserName(databaseUser);
    db.setPassword(databasePassword);
    db.setHostName(databaseHost);
    if (!db.open()) {
        fprintf(stderr, "Could not access database\n");
        return EXIT_FAILURE;
    }
    QDjango::setDatabase(db);

    // declare models
    QDjango::registerModel<User>();
    QDjango::registerModel<Group>();
    QDjango::registerModel<Message>();
    QDjango::registerModel<UserGroups>();

    QDjango::registerModel<File>();

    // run tests
    int errors = 0;

    for (int i = 0; i < count; ++i)
    {
        tst_QDjangoWhere testWhere;
        errors += QTest::qExec(&testWhere);

        tst_QDjangoCompiler testCompiler;
        errors += QTest::qExec(&testCompiler);

        tst_QDjangoMetaModel testMetaModel;
        errors += QTest::qExec(&testMetaModel);

        tst_QDjangoModel testModel;
        errors += QTest::qExec(&testModel);

        // field types
        tst_Bool testBool;
        errors += QTest::qExec(&testBool);

        tst_ByteArray testByteArray;
        errors += QTest::qExec(&testByteArray);

        tst_Date testDate;
        errors += QTest::qExec(&testDate);

        tst_DateTime testDateTime;
        errors += QTest::qExec(&testDateTime);

        tst_Double testDouble;
        errors += QTest::qExec(&testDouble);

        tst_Integer testInteger;
        errors += QTest::qExec(&testInteger);

        tst_LongLong testLongLong;
        errors += QTest::qExec(&testLongLong);

        tst_String testString;
        errors += QTest::qExec(&testString);

        tst_Time testTime;
        errors += QTest::qExec(&testTime);

        // models
        TestUser testUser;
        errors += QTest::qExec(&testUser);

        TestRelated testRelated;
        errors += QTest::qExec(&testRelated);

        TestShares testShares;
        errors += QTest::qExec(&testShares);

        // http
        TestHttp testHttp;
        errors += QTest::qExec(&testHttp);

        tst_QDjangoUrlResolver testUrlResolver;
        errors += QTest::qExec(&testUrlResolver);

        // script
        TestScript testScript;
        errors += QTest::qExec(&testScript);
    }

    if (errors)
    {
        qWarning() << "Total failed tests:" << errors;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
};

