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
#include "QDjangoQuerySet.h"
#include "QDjangoWhere.h"

#include "auth-models.h"
#include "util.h"

class Object : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString foo READ foo WRITE setFoo)
    Q_PROPERTY(int bar READ bar WRITE setBar)

    Q_CLASSINFO("__meta__", "db_table=foo_table")
    Q_CLASSINFO("foo", "max_length=255")
    Q_CLASSINFO("bar", "db_column=bar_column")

public:
    QString foo() const { return m_foo; };
    void setFoo(const QString &foo) { m_foo = foo; };

    int bar() const { return m_bar; };
    void setBar(int bar) { m_bar = bar; };

private:
    QString m_foo;
    int m_bar;
};

/** Test QDjangoQuerySetPrivate class.
 */
class tst_QDjangoQuerySetPrivate : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void countQuery();
    void deleteQuery();
    void insertQuery();
    void selectQuery();
    void updateQuery();
    void cleanupTestCase();

private:
    QDjangoMetaModel metaModel;
};

void tst_QDjangoQuerySetPrivate::initTestCase()
{
    QVERIFY(initialiseDatabase());

    metaModel = QDjango::registerModel<Object>();
    QCOMPARE(metaModel.createTable(), true);
}

void tst_QDjangoQuerySetPrivate::countQuery()
{
    QDjangoQuerySetPrivate qs("Object");
    qs.addFilter(QDjangoWhere("pk", QDjangoWhere::Equals, 1));
    QDjangoQuery query = qs.countQuery();

    QCOMPARE(normalizeSql(QDjango::database(), query.lastQuery()), QLatin1String("SELECT COUNT(*) FROM \"foo_table\" WHERE \"foo_table\".\"id\" = ?"));
    QCOMPARE(query.boundValues().size(), 1);
    QCOMPARE(query.boundValue(0), QVariant(1));
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
    data.insert("foo", "abc");

    QDjangoQuerySetPrivate qs("Object");
    QDjangoQuery query = qs.insertQuery(data);

    QCOMPARE(normalizeSql(QDjango::database(), query.lastQuery()), QLatin1String("INSERT INTO \"foo_table\" (\"foo\") VALUES(?)"));
    QCOMPARE(query.boundValues().size(), 1);
    QCOMPARE(query.boundValue(0), QVariant("abc"));
}

void tst_QDjangoQuerySetPrivate::selectQuery()
{
    QVariantMap data;
    data.insert("foo", "abc");

    {
        QDjangoQuerySetPrivate qs("Object");
        qs.addFilter(QDjangoWhere("pk", QDjangoWhere::Equals, 1));
        QDjangoQuery query = qs.selectQuery();

        QCOMPARE(normalizeSql(QDjango::database(), query.lastQuery()), QLatin1String("SELECT \"foo_table\".\"id\", \"foo_table\".\"foo\", \"foo_table\".\"bar_column\" FROM \"foo_table\" WHERE \"foo_table\".\"id\" = ?"));
        QCOMPARE(query.boundValues().size(), 1);
        QCOMPARE(query.boundValue(0), QVariant(1));
    }

    {
        QDjangoQuerySetPrivate qs("Object");
        qs.addFilter(QDjangoWhere("bar", QDjangoWhere::Equals, 3));
        QDjangoQuery query = qs.selectQuery();

        QCOMPARE(normalizeSql(QDjango::database(), query.lastQuery()), QLatin1String("SELECT \"foo_table\".\"id\", \"foo_table\".\"foo\", \"foo_table\".\"bar_column\" FROM \"foo_table\" WHERE \"foo_table\".\"bar_column\" = ?"));
        QCOMPARE(query.boundValues().size(), 1);
        QCOMPARE(query.boundValue(0), QVariant(3));
    }
}

void tst_QDjangoQuerySetPrivate::updateQuery()
{
    QVariantMap data;
    data.insert("foo", "abc");

    {
        QDjangoQuerySetPrivate qs("Object");
        qs.addFilter(QDjangoWhere("pk", QDjangoWhere::Equals, 1));
        QDjangoQuery query = qs.updateQuery(data);

        QCOMPARE(normalizeSql(QDjango::database(), query.lastQuery()), QLatin1String("UPDATE \"foo_table\" SET \"foo\" = ? WHERE \"foo_table\".\"id\" = ?"));
        QCOMPARE(query.boundValues().size(), 2);
        QCOMPARE(query.boundValue(0), QVariant("abc"));
        QCOMPARE(query.boundValue(1), QVariant(1));
    }

    {
        QDjangoQuerySetPrivate qs("Object");
        qs.addFilter(QDjangoWhere("bar", QDjangoWhere::Equals, 3));
        QDjangoQuery query = qs.updateQuery(data);

        QCOMPARE(normalizeSql(QDjango::database(), query.lastQuery()), QLatin1String("UPDATE \"foo_table\" SET \"foo\" = ? WHERE \"foo_table\".\"bar_column\" = ?"));
        QCOMPARE(query.boundValues().size(), 2);
        QCOMPARE(query.boundValue(0), QVariant("abc"));
        QCOMPARE(query.boundValue(1), QVariant(3));
    }
}

void tst_QDjangoQuerySetPrivate::cleanupTestCase()
{
    metaModel.dropTable();
}

QTEST_MAIN(tst_QDjangoQuerySetPrivate)
#include "tst_qdjangoqueryset.moc"
