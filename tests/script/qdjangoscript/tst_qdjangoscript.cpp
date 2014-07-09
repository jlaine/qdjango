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

#include "QDjangoScript.h"

#include "auth-models.h"
#include "util.h"

Q_DECLARE_METATYPE(QDjangoQuerySet<User>)

/** Test QDjango scripting.
 */
class tst_QDjangoScript : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void testWhereConstructor();
    void testWhereOperators();
    void testWhereToString();
    void testModel();
    void cleanupTestCase();

private:
    QDjangoMetaModel metaModel;
    QScriptEngine *engine;
};

void tst_QDjangoScript::cleanupTestCase()
{
    metaModel.dropTable();

    delete engine;
}

void tst_QDjangoScript::initTestCase()
{
    initialiseDatabase();

    metaModel = QDjango::registerModel<User>();
    QCOMPARE(metaModel.createTable(), true);

    engine = new QScriptEngine(this);
    QDjangoScript::registerWhere(engine);
    QDjangoScript::registerModel<User>(engine);
}

void tst_QDjangoScript::testWhereConstructor()
{
    QScriptValue result;
    QDjangoWhere where;

    // equals
    result = engine->evaluate("Q({'username': 'foobar'})");
    where = engine->fromScriptValue<QDjangoWhere>(result);
    CHECKWHERE(where, QLatin1String("username = ?"), QVariantList() << "foobar");

    // less than
    result = engine->evaluate("Q({'username__lt': 'foobar'})");
    where = engine->fromScriptValue<QDjangoWhere>(result);
    CHECKWHERE(where, QLatin1String("username < ?"), QVariantList() << "foobar");

    // less than or equal to
    result = engine->evaluate("Q({'username__lte': 'foobar'})");
    where = engine->fromScriptValue<QDjangoWhere>(result);
    CHECKWHERE(where, QLatin1String("username <= ?"), QVariantList() << "foobar");

    // greater than
    result = engine->evaluate("Q({'username__gt': 'foobar'})");
    where = engine->fromScriptValue<QDjangoWhere>(result);
    CHECKWHERE(where, QLatin1String("username > ?"), QVariantList() << "foobar");

    // greater than or equal to
    result = engine->evaluate("Q({'username__gte': 'foobar'})");
    where = engine->fromScriptValue<QDjangoWhere>(result);
    CHECKWHERE(where, QLatin1String("username >= ?"), QVariantList() << "foobar");

    if (QDjango::database().driverName() == QLatin1String("QMYSQL")) {
        // starts with
        result = engine->evaluate("Q({'username__startswith': 'foobar'})");
        where = engine->fromScriptValue<QDjangoWhere>(result);
        CHECKWHERE(where, QLatin1String("username LIKE BINARY ?"), QVariantList() << "foobar%");

        // ends with
        result = engine->evaluate("Q({'username__endswith': 'foobar'})");
        where = engine->fromScriptValue<QDjangoWhere>(result);
        CHECKWHERE(where, QLatin1String("username LIKE BINARY ?"), QVariantList() << "%foobar");

        // contains
        result = engine->evaluate("Q({'username__contains': 'foobar'})");
        where = engine->fromScriptValue<QDjangoWhere>(result);
        CHECKWHERE(where, QLatin1String("username LIKE BINARY ?"), QVariantList() << "%foobar%");
    } else {
        // starts with
        result = engine->evaluate("Q({'username__startswith': 'foobar'})");
        where = engine->fromScriptValue<QDjangoWhere>(result);
        CHECKWHERE(where, QLatin1String("username LIKE ?"), QVariantList() << "foobar%");

        // ends with
        result = engine->evaluate("Q({'username__endswith': 'foobar'})");
        where = engine->fromScriptValue<QDjangoWhere>(result);
        CHECKWHERE(where, QLatin1String("username LIKE ?"), QVariantList() << "%foobar");

        // contains
        result = engine->evaluate("Q({'username__contains': 'foobar'})");
        where = engine->fromScriptValue<QDjangoWhere>(result);
        CHECKWHERE(where, QLatin1String("username LIKE ?"), QVariantList() << "%foobar%");
    }

    // in
    result = engine->evaluate("Q({'username__in': ['foobar', 'wiz']})");
    where = engine->fromScriptValue<QDjangoWhere>(result);
    CHECKWHERE(where, QLatin1String("username IN (?, ?)"), QVariantList() << "foobar" << "wiz");

    // double constructor
    result = engine->evaluate("Q(Q({'username': 'foobar'}))");
    where = engine->fromScriptValue<QDjangoWhere>(result);
    CHECKWHERE(where, QLatin1String("username = ?"), QVariantList() << "foobar");
}

void tst_QDjangoScript::testWhereOperators()
{
    QScriptValue result;
    QDjangoWhere where;

    // AND operator
    result = engine->evaluate("Q({'username': 'foobar'}).and(Q({'password': 'foopass'}))");
    where = engine->fromScriptValue<QDjangoWhere>(result);
    CHECKWHERE(where, QLatin1String("username = ? AND password = ?"), QVariantList() << "foobar" << "foopass");

    // OR operator
    result = engine->evaluate("Q({'username': 'foobar'}).or(Q({'password': 'foopass'}))");
    where = engine->fromScriptValue<QDjangoWhere>(result);
    CHECKWHERE(where, QLatin1String("username = ? OR password = ?"), QVariantList() << "foobar" << "foopass");
}

void tst_QDjangoScript::testWhereToString()
{
    QScriptValue result = engine->evaluate("Q({'username': 'foobar'}).toString()");
    QCOMPARE(result.toString(), QLatin1String("Q(username = ?)"));
}

void tst_QDjangoScript::testModel()
{
    // create model instance
    QScriptValue result = engine->evaluate("user = User();");
    User *user = qobject_cast<User*>(result.toQObject());
    QVERIFY(user != 0);

    // set properties
    engine->evaluate("user.username = 'foobar';");
    QCOMPARE(user->username(), QLatin1String("foobar"));
}

QTEST_MAIN(tst_QDjangoScript)
#include "tst_qdjangoscript.moc"
