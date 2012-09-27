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
#include "QDjango_p.h"
#include "QDjangoModel.h"
#include "QDjangoQuerySet.h"
#include "QDjangoWhere.h"

#include "tst_qdjangometamodel.h"
#include "util.h"

#define Q QDjangoWhere

template<class T>
void init(const QStringList &sql)
{
    const QDjangoMetaModel metaModel = QDjango::registerModel<T>();
    QCOMPARE(metaModel.createTableSql(), sql);
    QCOMPARE(metaModel.createTable(), true);
}

template<class T, class K>
void setAndGet(const K &value)
{
    // save object
    T v1;
    v1.setValue(value);
    QCOMPARE(v1.save(), true);
    QVERIFY(!v1.pk().isNull());

    // save again
    QCOMPARE(v1.save(), true);

    // get object
    T v2;
    QVERIFY(QDjangoQuerySet<T>().get(Q(QLatin1String("pk"), Q::Equals, v1.pk()), &v2) != 0);
    QCOMPARE(v2.value(), value);
}

template<class T>
void cleanup()
{
    const QDjangoMetaModel metaModel = QDjango::registerModel<T>();
    QCOMPARE(metaModel.dropTable(), true);
}

tst_FkConstraint::tst_FkConstraint(QObject *parent)
    : QDjangoModel(parent)
{
    setForeignKey("noConstraint", new User(this));
    setForeignKey("cascadeConstraint", new User(this));
    setForeignKey("restrictConstraint", new User(this));
    setForeignKey("nullConstraint", new User(this));
}

User *tst_FkConstraint::noConstraint() const
{
    return qobject_cast<User*>(foreignKey("noConstraint"));
}

void tst_FkConstraint::setNoConstraint(User *user)
{
    setForeignKey("noConstraint", user);
}

User *tst_FkConstraint::cascadeConstraint() const
{
    return qobject_cast<User*>(foreignKey("cascadeConstraint"));
}

void tst_FkConstraint::setCascadeConstraint(User *user)
{
    setForeignKey("cascadeConstraint", user);
}

User *tst_FkConstraint::restrictConstraint() const
{
    return qobject_cast<User*>(foreignKey("restrictConstraint"));
}

void tst_FkConstraint::setRestrictConstraint(User *user)
{
    setForeignKey("restrictConstraint", user);
}

User *tst_FkConstraint::nullConstraint() const
{
    return qobject_cast<User*>(foreignKey("nullConstraint"));
}

void tst_FkConstraint::setNullConstraint(User *user)
{
    setForeignKey("nullConstraint", user);
}

void tst_QDjangoMetaModel::initTestCase()
{
    QVERIFY(initialiseDatabase());
}

void tst_QDjangoMetaModel::testBool()
{
    QStringList sql;
    if (QDjango::database().driverName() == QLatin1String("QPSQL"))
        sql << QLatin1String("CREATE TABLE \"tst_bool\" (\"id\" serial PRIMARY KEY, \"value\" boolean NOT NULL)");
    else
        sql << QLatin1String("CREATE TABLE \"tst_bool\" (\"id\" integer NOT NULL PRIMARY KEY AUTOINCREMENT, \"value\" bool NOT NULL)");

    init<tst_Bool>(sql);
    setAndGet<tst_Bool>(true);
    setAndGet<tst_Bool>(false);
    cleanup<tst_Bool>();
}

void tst_QDjangoMetaModel::testByteArray()
{
    QStringList sql;
    if (QDjango::database().driverName() == QLatin1String("QPSQL"))
        sql << QLatin1String("CREATE TABLE \"tst_bytearray\" (\"id\" serial PRIMARY KEY, \"value\" bytea NOT NULL)");
    else
        sql << QLatin1String("CREATE TABLE \"tst_bytearray\" (\"id\" integer NOT NULL PRIMARY KEY AUTOINCREMENT, \"value\" blob NOT NULL)");

    init<tst_ByteArray>(sql);
    setAndGet<tst_ByteArray>(QByteArray("01234567", 8));
    setAndGet<tst_ByteArray>(QByteArray("\x00\x01\x02\x03\x04\x05\x06\x07", 8));
    cleanup<tst_ByteArray>();
}

void tst_QDjangoMetaModel::testDate()
{
    QStringList sql;
    if (QDjango::database().driverName() == QLatin1String("QPSQL"))
        sql << QLatin1String("CREATE TABLE \"tst_date\" (\"id\" serial PRIMARY KEY, \"value\" date NOT NULL)");
    else
        sql << QLatin1String("CREATE TABLE \"tst_date\" (\"id\" integer NOT NULL PRIMARY KEY AUTOINCREMENT, \"value\" date NOT NULL)");

    init<tst_Date>(sql);
    setAndGet<tst_Date>(QDate(2012, 1, 8));
    cleanup<tst_Date>();
}

void tst_QDjangoMetaModel::testDateTime()
{
    QStringList sql;
    if (QDjango::database().driverName() == QLatin1String("QPSQL"))
        sql << QLatin1String("CREATE TABLE \"tst_datetime\" (\"id\" serial PRIMARY KEY, \"value\" timestamp NOT NULL)");
    else
        sql << QLatin1String("CREATE TABLE \"tst_datetime\" (\"id\" integer NOT NULL PRIMARY KEY AUTOINCREMENT, \"value\" datetime NOT NULL)");

    init<tst_DateTime>(sql);
    setAndGet<tst_DateTime>(QDateTime(QDate(2012, 1, 8), QTime(3, 4, 5)));
    cleanup<tst_DateTime>();
}

void tst_QDjangoMetaModel::testDouble()
{
    QStringList sql;
    if (QDjango::database().driverName() == QLatin1String("QPSQL"))
        sql << QLatin1String("CREATE TABLE \"tst_double\" (\"id\" serial PRIMARY KEY, \"value\" real NOT NULL)");
    else
        sql << QLatin1String("CREATE TABLE \"tst_double\" (\"id\" integer NOT NULL PRIMARY KEY AUTOINCREMENT, \"value\" real NOT NULL)");

    init<tst_Double>(sql);
    setAndGet<tst_Double>(double(3.14159));;
    cleanup<tst_Double>();
}

void tst_QDjangoMetaModel::testInteger()
{
    QStringList sql;
    if (QDjango::database().driverName() == QLatin1String("QPSQL"))
        sql << QLatin1String("CREATE TABLE \"tst_integer\" (\"id\" serial PRIMARY KEY, \"value\" integer NOT NULL)");
    else
        sql << QLatin1String("CREATE TABLE \"tst_integer\" (\"id\" integer NOT NULL PRIMARY KEY AUTOINCREMENT, \"value\" integer NOT NULL)");

    init<tst_Integer>(sql);
    setAndGet<tst_Integer>(0);
    setAndGet<tst_Integer>(-2147483647);
    setAndGet<tst_Integer>(2147483647);
    cleanup<tst_Integer>();
}

void tst_QDjangoMetaModel::testLongLong()
{
    QStringList sql;
    if (QDjango::database().driverName() == QLatin1String("QPSQL"))
        sql << QLatin1String("CREATE TABLE \"tst_longlong\" (\"id\" serial PRIMARY KEY, \"value\" bigint NOT NULL)");
    else
        sql << QLatin1String("CREATE TABLE \"tst_longlong\" (\"id\" integer NOT NULL PRIMARY KEY AUTOINCREMENT, \"value\" bigint NOT NULL)");

    init<tst_LongLong>(sql);
    setAndGet<tst_LongLong>(qlonglong(0));
    setAndGet<tst_LongLong>(qlonglong(-9223372036854775807ll));
    setAndGet<tst_LongLong>(qlonglong(9223372036854775807ll));
    cleanup<tst_LongLong>();
}

void tst_QDjangoMetaModel::testString()
{
    QStringList sql;
    if (QDjango::database().driverName() == QLatin1String("QPSQL"))
        sql << QLatin1String("CREATE TABLE \"tst_string\" (\"id\" serial PRIMARY KEY, \"value\" varchar(255) NOT NULL)");
    else
        sql << QLatin1String("CREATE TABLE \"tst_string\" (\"id\" integer NOT NULL PRIMARY KEY AUTOINCREMENT, \"value\" varchar(255) NOT NULL)");

    init<tst_String>(sql);
    setAndGet<tst_String>(QLatin1String("foo bar"));
    cleanup<tst_String>();
}

void tst_QDjangoMetaModel::testTime()
{
    QStringList sql;
    if (QDjango::database().driverName() == QLatin1String("QPSQL"))
        sql << QLatin1String("CREATE TABLE \"tst_time\" (\"id\" serial PRIMARY KEY, \"value\" time NOT NULL)");
    else
        sql << QLatin1String("CREATE TABLE \"tst_time\" (\"id\" integer NOT NULL PRIMARY KEY AUTOINCREMENT, \"value\" time NOT NULL)");

    init<tst_Time>(sql);
    setAndGet<tst_Time>(QTime(3, 4, 5));
    cleanup<tst_Time>();
}

void tst_QDjangoMetaModel::testOptions()
{
    QStringList sql;
    if (QDjango::database().driverName() == QLatin1String("QPSQL"))
        sql << QLatin1String(
            "CREATE TABLE \"some_table\" ("
                "\"id\" serial PRIMARY KEY, "
                "\"aField\" integer NOT NULL, "
                "\"b_field\" integer NOT NULL, "
                "\"indexField\" integer NOT NULL, "
                "\"nullField\" integer, "
                "\"uniqueField\" integer NOT NULL UNIQUE, "
                "UNIQUE (\"aField\", \"b_field\")"
            ")");
    else
        sql << QLatin1String(
            "CREATE TABLE \"some_table\" ("
                "\"id\" integer NOT NULL PRIMARY KEY AUTOINCREMENT, "
                "\"aField\" integer NOT NULL, "
                "\"b_field\" integer NOT NULL, "
                "\"indexField\" integer NOT NULL, "
                "\"nullField\" integer, "
                "\"uniqueField\" integer NOT NULL UNIQUE, "
                "UNIQUE (\"aField\", \"b_field\")"
            ")");

    sql << QLatin1String("CREATE INDEX \"some_table_ac243651\" ON \"some_table\" (\"indexField\")");
    init<tst_Options>(sql);
    cleanup<tst_Options>();
}

/** Test foreign key constraint sql generation
 */
void tst_QDjangoMetaModel::testConstraints()
{
    QStringList sql;
    if (QDjango::database().driverName() == QLatin1String("QPSQL"))
        sql << QLatin1String("CREATE TABLE \"tst_fkconstraint\" ("
            "\"id\" serial PRIMARY KEY, "
            "\"noConstraint_id\" integer NOT NULL REFERENCES \"user\" (\"id\") DEFERRABLE INITIALLY DEFERRED, "
            "\"cascadeConstraint_id\" integer NOT NULL REFERENCES \"user\" (\"id\") DEFERRABLE INITIALLY DEFERRED ON DELETE CASCADE, "
            "\"restrictConstraint_id\" integer NOT NULL REFERENCES \"user\" (\"id\") DEFERRABLE INITIALLY DEFERRED ON DELETE RESTRICT, "
            "\"nullConstraint_id\" integer REFERENCES \"user\" (\"id\") DEFERRABLE INITIALLY DEFERRED ON DELETE SET NULL"
            ")");
    else
       sql << QLatin1String("CREATE TABLE \"tst_fkconstraint\" ("
            "\"id\" integer NOT NULL PRIMARY KEY AUTOINCREMENT, "
            "\"noConstraint_id\" integer NOT NULL REFERENCES \"user\" (\"id\"), "
            "\"cascadeConstraint_id\" integer NOT NULL REFERENCES \"user\" (\"id\") ON DELETE CASCADE, "
            "\"restrictConstraint_id\" integer NOT NULL REFERENCES \"user\" (\"id\") ON DELETE RESTRICT, "
            "\"nullConstraint_id\" integer REFERENCES \"user\" (\"id\") ON DELETE SET NULL"
            ")");
    sql << QLatin1String("CREATE INDEX \"tst_fkconstraint_f388fc3c\" ON \"tst_fkconstraint\" (\"noConstraint_id\")");
    sql << QLatin1String("CREATE INDEX \"tst_fkconstraint_4634d592\" ON \"tst_fkconstraint\" (\"cascadeConstraint_id\")");
    sql << QLatin1String("CREATE INDEX \"tst_fkconstraint_728cefe1\" ON \"tst_fkconstraint\" (\"restrictConstraint_id\")");
    sql << QLatin1String("CREATE INDEX \"tst_fkconstraint_44c71620\" ON \"tst_fkconstraint\" (\"nullConstraint_id\")");

    QDjango::registerModel<User>();
    QDjangoMetaModel metaModel = QDjango::registerModel<tst_FkConstraint>();
    QCOMPARE(metaModel.createTableSql(), sql);
}

QTEST_MAIN(tst_QDjangoMetaModel)
