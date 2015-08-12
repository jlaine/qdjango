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

#include "QDjangoWhere.h"

#include "util.h"

/** Test QDjangoWhere class.
 */
class tst_QDjangoWhere : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void emptyWhere();
    void equalsWhere();
    void iEqualsWhere();
    void notEqualsWhere();
    void iNotEqualsWhere();
    void greaterThan();
    void greaterOrEquals();
    void lessThan();
    void lessOrEquals();
    void isIn();
    void isNull();
    void startsWith();
    void iStartsWith();
    void endsWith();
    void iEndsWith();
    void contains();
    void iContains();
    void andWhere();
    void orWhere();
    void complexWhere();
    void toString();
};

void tst_QDjangoWhere::initTestCase()
{
    QVERIFY(initialiseDatabase());
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

/** Test case-insensitive "=" comparison.
  */
void tst_QDjangoWhere::iEqualsWhere()
{
    QDjangoDatabase::DatabaseType databaseType = QDjangoDatabase::databaseType(QDjango::database());
     if (databaseType == QDjangoDatabase::PostgreSQL) {
        QDjangoWhere testQuery = QDjangoWhere("name", QDjangoWhere::IEquals, "abc");
        CHECKWHERE(testQuery, QLatin1String("UPPER(name::text) LIKE UPPER(?)"), QVariantList() << "abc");

        testQuery = !QDjangoWhere("name", QDjangoWhere::IEquals, "abc");
        CHECKWHERE(testQuery, QLatin1String("UPPER(name::text) NOT LIKE UPPER(?)"), QVariantList() << "abc");
    } else {
         QDjangoWhere testQuery = QDjangoWhere("name", QDjangoWhere::IEquals, "abc");
         CHECKWHERE(testQuery, QLatin1String("name LIKE ?"), QVariantList() << "abc");

         testQuery = !QDjangoWhere("name", QDjangoWhere::IEquals, "abc");
         CHECKWHERE(testQuery, QLatin1String("name NOT LIKE ?"), QVariantList() << "abc");
     }
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

/** Test case-insensitive "!=" comparison.
  */
void tst_QDjangoWhere::iNotEqualsWhere()
{
    QDjangoDatabase::DatabaseType databaseType = QDjangoDatabase::databaseType(QDjango::database());
    if (databaseType == QDjangoDatabase::PostgreSQL) {
        QDjangoWhere testQuery = QDjangoWhere("name", QDjangoWhere::INotEquals, "abc");
        CHECKWHERE(testQuery, QLatin1String("UPPER(name::text) NOT LIKE UPPER(?)"), QVariantList() << "abc");

        testQuery = !QDjangoWhere("name", QDjangoWhere::INotEquals, "abc");
        CHECKWHERE(testQuery, QLatin1String("UPPER(name::text) LIKE UPPER(?)"), QVariantList() << "abc");
    } else {
        QDjangoWhere testQuery = QDjangoWhere("name", QDjangoWhere::INotEquals, "abc");
        CHECKWHERE(testQuery, QLatin1String("name NOT LIKE ?"), QVariantList() << "abc");

        testQuery = !QDjangoWhere("name", QDjangoWhere::INotEquals, "abc");
        CHECKWHERE(testQuery, QLatin1String("name LIKE ?"), QVariantList() << "abc");
    }
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
    QDjangoDatabase::DatabaseType databaseType = QDjangoDatabase::databaseType(QDjango::database());
    if (databaseType == QDjangoDatabase::MySqlServer) {
        QDjangoWhere testQuery = QDjangoWhere("name", QDjangoWhere::StartsWith, "abc");
        CHECKWHERE(testQuery, QLatin1String("name LIKE BINARY ?"), QVariantList() << "abc%");

        testQuery = !QDjangoWhere("name", QDjangoWhere::StartsWith, "abc");
        CHECKWHERE(testQuery, QLatin1String("name NOT LIKE BINARY ?"), QVariantList() << "abc%");
    } else {
        QDjangoWhere testQuery = QDjangoWhere("name", QDjangoWhere::StartsWith, "abc");
        CHECKWHERE(testQuery, QLatin1String("name LIKE ?"), QVariantList() << "abc%");

        testQuery = !QDjangoWhere("name", QDjangoWhere::StartsWith, "abc");
        CHECKWHERE(testQuery, QLatin1String("name NOT LIKE ?"), QVariantList() << "abc%");
    }

}

/** Test "istartswith" comparison.
  */

void tst_QDjangoWhere::iStartsWith()
{
    QDjangoDatabase::DatabaseType databaseType = QDjangoDatabase::databaseType(QDjango::database());
    if (databaseType == QDjangoDatabase::PostgreSQL) {
        QDjangoWhere testQuery = QDjangoWhere("name", QDjangoWhere::IStartsWith, "abc");
        CHECKWHERE(testQuery, QLatin1String("UPPER(name::text) LIKE UPPER(?)"), QVariantList() << "abc%");

        testQuery = !QDjangoWhere("name", QDjangoWhere::IStartsWith, "abc");
        CHECKWHERE(testQuery, QLatin1String("UPPER(name::text) NOT LIKE UPPER(?)"), QVariantList() << "abc%");
    } else {
        QDjangoWhere testQuery = QDjangoWhere("name", QDjangoWhere::IStartsWith, "abc");
        CHECKWHERE(testQuery, QLatin1String("name LIKE ?"), QVariantList() << "abc%");

        testQuery = !QDjangoWhere("name", QDjangoWhere::IStartsWith, "abc");
        CHECKWHERE(testQuery, QLatin1String("name NOT LIKE ?"), QVariantList() << "abc%");
    }
}

/** Test "endswith" comparison.
 */
void tst_QDjangoWhere::endsWith()
{
    QDjangoDatabase::DatabaseType databaseType = QDjangoDatabase::databaseType(QDjango::database());
    if (databaseType == QDjangoDatabase::MySqlServer) {
        QDjangoWhere testQuery = QDjangoWhere("name", QDjangoWhere::EndsWith, "abc");
        CHECKWHERE(testQuery, QLatin1String("name LIKE BINARY ?"), QVariantList() << "%abc");

        testQuery = !QDjangoWhere("name", QDjangoWhere::EndsWith, "abc");
        CHECKWHERE(testQuery, QLatin1String("name NOT LIKE BINARY ?"), QVariantList() << "%abc");
    } else {
        QDjangoWhere testQuery = QDjangoWhere("name", QDjangoWhere::EndsWith, "abc");
        CHECKWHERE(testQuery, QLatin1String("name LIKE ?"), QVariantList() << "%abc");

        testQuery = !QDjangoWhere("name", QDjangoWhere::EndsWith, "abc");
        CHECKWHERE(testQuery, QLatin1String("name NOT LIKE ?"), QVariantList() << "%abc");
    }
}

/** Test "iendswith" comparison.
  */

void tst_QDjangoWhere::iEndsWith()
{
    QDjangoDatabase::DatabaseType databaseType = QDjangoDatabase::databaseType(QDjango::database());
    if (databaseType == QDjangoDatabase::PostgreSQL) {
        QDjangoWhere testQuery = QDjangoWhere("name", QDjangoWhere::IEndsWith, "abc");
        CHECKWHERE(testQuery, QLatin1String("UPPER(name::text) LIKE UPPER(?)"), QVariantList() << "%abc");

        testQuery = !QDjangoWhere("name", QDjangoWhere::IEndsWith, "abc");
        CHECKWHERE(testQuery, QLatin1String("UPPER(name::text) NOT LIKE UPPER(?)"), QVariantList() << "%abc");
    } else {
        QDjangoWhere testQuery = QDjangoWhere("name", QDjangoWhere::IEndsWith, "abc");
        CHECKWHERE(testQuery, QLatin1String("name LIKE ?"), QVariantList() << "%abc");

        testQuery = !QDjangoWhere("name", QDjangoWhere::IEndsWith, "abc");
        CHECKWHERE(testQuery, QLatin1String("name NOT LIKE ?"), QVariantList() << "%abc");
    }

}


/** Test "contains" comparison.
 */
void tst_QDjangoWhere::contains()
{
    QDjangoDatabase::DatabaseType databaseType = QDjangoDatabase::databaseType(QDjango::database());
    if (databaseType == QDjangoDatabase::MySqlServer) {
        QDjangoWhere testQuery = QDjangoWhere("name", QDjangoWhere::Contains, "abc");
        CHECKWHERE(testQuery, QLatin1String("name LIKE BINARY ?"), QVariantList() << "%abc%");

        testQuery = !QDjangoWhere("name", QDjangoWhere::Contains, "abc");
        CHECKWHERE(testQuery, QLatin1String("name NOT LIKE BINARY ?"), QVariantList() << "%abc%");
    } else {
        QDjangoWhere testQuery = QDjangoWhere("name", QDjangoWhere::Contains, "abc");
        CHECKWHERE(testQuery, QLatin1String("name LIKE ?"), QVariantList() << "%abc%");

        testQuery = !QDjangoWhere("name", QDjangoWhere::Contains, "abc");
        CHECKWHERE(testQuery, QLatin1String("name NOT LIKE ?"), QVariantList() << "%abc%");
    }
}

/** Test "icontains" comparison"
  */

void tst_QDjangoWhere::iContains()
{
    QDjangoDatabase::DatabaseType databaseType = QDjangoDatabase::databaseType(QDjango::database());
    if (databaseType == QDjangoDatabase::PostgreSQL) {
        QDjangoWhere testQuery = QDjangoWhere("name", QDjangoWhere::IContains, "abc");
        CHECKWHERE(testQuery, QLatin1String("UPPER(name::text) LIKE UPPER(?)"), QVariantList() << "%abc%");

        testQuery = !QDjangoWhere("name", QDjangoWhere::IContains, "abc");
        CHECKWHERE(testQuery, QLatin1String("UPPER(name::text) NOT LIKE UPPER(?)"), QVariantList() << "%abc%");
    } else {
        QDjangoWhere testQuery = QDjangoWhere("name", QDjangoWhere::IContains, "abc");
        CHECKWHERE(testQuery, QLatin1String("name LIKE ?"), QVariantList() << "%abc%");

        testQuery = !QDjangoWhere("name", QDjangoWhere::IContains, "abc");
        CHECKWHERE(testQuery, QLatin1String("name NOT LIKE ?"), QVariantList() << "%abc%");
    }
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

    testQuery = queryId && queryUsername && queryPassword;
    CHECKWHERE(testQuery, QLatin1String("id = ? AND username = ? AND password = ?"), QVariantList() << 1 << "foouser" << "foopass");

    testQuery = queryId || queryUsername || queryPassword;
    CHECKWHERE(testQuery, QLatin1String("id = ? OR username = ? OR password = ?"), QVariantList() << 1 << "foouser" << "foopass");
}

void tst_QDjangoWhere::toString()
{
    QDjangoWhere testQuery;

    const QDjangoWhere queryId("id", QDjangoWhere::Equals, 1);
    const QDjangoWhere queryUsername("username", QDjangoWhere::Equals, "foo");

    testQuery = queryId;
    QCOMPARE(testQuery.toString(), QLatin1String("QDjangoWhere(key=\"id\", operation=\"Equals\", value=\"1\", negate=false)"));

    testQuery = queryId || queryUsername;
    QCOMPARE(testQuery.toString(), QLatin1String("QDjangoWhere(key=\"id\", operation=\"Equals\", value=\"1\", negate=false) || QDjangoWhere(key=\"username\", operation=\"Equals\", value=\"foo\", negate=false)"));

    testQuery = queryId && queryUsername;
    QCOMPARE(testQuery.toString(), QLatin1String("QDjangoWhere(key=\"id\", operation=\"Equals\", value=\"1\", negate=false) && QDjangoWhere(key=\"username\", operation=\"Equals\", value=\"foo\", negate=false)"));
}

QTEST_MAIN(tst_QDjangoWhere)
#include "tst_qdjangowhere.moc"
