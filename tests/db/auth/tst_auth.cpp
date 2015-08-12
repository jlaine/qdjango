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

#include "QDjangoQuerySet.h"
#include "QDjangoWhere.h"

#include "auth-models.h"
#include "util.h"

/** Tests for the User class.
 */
class tst_Auth: public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void create();
    void remove();
    void removeFilter();
    void removeLimit();
    void get();
    void filter();
    void filterLike();
    void exclude();
    void limit();
    void subLimit();
    void orderBy();
    void update();
    void values();
    void valuesList();
    void constIterator();
    void testGroups();
    void testRelated();
    void filterRelated();
    void cleanup();
    void cleanupTestCase();

private:
    void loadFixtures();
};

/** Create database table before running tests.
 */
void tst_Auth::initTestCase()
{
    QVERIFY(initialiseDatabase());
    QDjango::registerModel<User>();
    QDjango::registerModel<Group>();
    QDjango::registerModel<Message>();
    QDjango::registerModel<UserGroups>();
    QVERIFY(QDjango::createTables());
}

/** Load fixtures consisting of 3 users.
 */
void tst_Auth::loadFixtures()
{
    User foo;
    foo.setUsername("foouser");
    foo.setPassword("foopass");
    foo.setLastLogin(QDateTime(QDate(2010, 6, 1), QTime(10, 5, 14)));
    QCOMPARE(foo.save(), true);

    User bar;
    bar.setUsername("baruser");
    bar.setPassword("barpass");
    bar.setLastLogin(QDateTime(QDate(2010, 6, 1), QTime(10, 6, 31)));
    QCOMPARE(bar.save(), true);

    User wiz;
    wiz.setUsername("wizuser");
    wiz.setPassword("wizpass");
    wiz.setLastLogin(QDateTime(QDate(2010, 6, 1), QTime(10, 7, 18)));
    QCOMPARE(wiz.save(), true);

    QCOMPARE(QDjangoQuerySet<User>().count(), 3);
    QCOMPARE(QDjangoQuerySet<User>().size(), 3);
}

void tst_Auth::create()
{
    const QDjangoQuerySet<User> users;
    User *other;

    // create
    User user;
    user.setUsername("foouser");
    user.setPassword("foopass");
    user.setLastLogin(QDateTime(QDate(2010, 6, 1), QTime(10, 5, 14)));
    QCOMPARE(user.save(), true);
    QCOMPARE(users.all().size(), 1);

    // get by id
    other = users.get(QDjangoWhere("id", QDjangoWhere::Equals, 1));
    QVERIFY(other != 0);
    QCOMPARE(other->pk(), QVariant(1));
    QCOMPARE(other->username(), QLatin1String("foouser"));
    QCOMPARE(other->password(), QLatin1String("foopass"));
    delete other;

    // get by pk
    other = users.get(QDjangoWhere("pk", QDjangoWhere::Equals, 1));
    QVERIFY(other != 0);
    QCOMPARE(other->pk(), QVariant(1));
    QCOMPARE(other->username(), QLatin1String("foouser"));
    QCOMPARE(other->password(), QLatin1String("foopass"));
    delete other;

    // get by username
    other = users.get(QDjangoWhere("username", QDjangoWhere::Equals, "foouser"));
    QVERIFY(other != 0);
    QCOMPARE(other->pk(), QVariant(1));
    QCOMPARE(other->username(), QLatin1String("foouser"));
    QCOMPARE(other->password(), QLatin1String("foopass"));
    QCOMPARE(other->lastLogin(), QDateTime(QDate(2010, 6, 1), QTime(10, 5, 14)));
    delete other;

    // update
    user.setPassword("foopass2");
    QCOMPARE(user.save(), true);
    QCOMPARE(users.all().size(), 1);

    other = users.get(QDjangoWhere("username", QDjangoWhere::Equals, "foouser"));
    QVERIFY(other != 0);
    QCOMPARE(other->pk(), QVariant(1));
    QCOMPARE(other->username(), QLatin1String("foouser"));
    QCOMPARE(other->password(), QLatin1String("foopass2"));
    delete other;

    User user2;
    user2.setUsername("baruser");
    user2.setPassword("barpass");
    user2.setLastLogin(QDateTime(QDate(2010, 6, 1), QTime(10, 6, 31)));
    QCOMPARE(user2.save(), true);
    QCOMPARE(users.all().size(), 2);

    other = users.get(QDjangoWhere("username", QDjangoWhere::Equals, "baruser"));
    QVERIFY(other != 0);
    QCOMPARE(other->pk(), QVariant(2));
    QCOMPARE(other->username(), QLatin1String("baruser"));
    QCOMPARE(other->password(), QLatin1String("barpass"));
    QCOMPARE(other->lastLogin(), QDateTime(QDate(2010, 6, 1), QTime(10, 6, 31)));
    delete other;
}

/** Test removing a single user.
 */
void tst_Auth::remove()
{
    const QDjangoQuerySet<User> users;

    User user;
    user.setUsername("foouser");
    user.setPassword("foopass");

    QCOMPARE(user.save(), true);
    QCOMPARE(users.all().size(), 1);

    QCOMPARE(user.remove(), true);
    QCOMPARE(users.all().size(), 0);
}

/** Test removing multiple users.
  */
void tst_Auth::removeFilter()
{
    loadFixtures();

    // remove "foouser" and "baruser"
    const QDjangoQuerySet<User> users;
    QDjangoQuerySet<User> qs = users.filter(QDjangoWhere("username", QDjangoWhere::IsIn, QStringList() << "foouser" << "baruser"));
    QCOMPARE(qs.remove(), true);

    // check remaining user
    qs = users.all();
    QCOMPARE(qs.size(), 1);
    User *other = qs.at(0);
    QVERIFY(other != 0);
    QCOMPARE(other->username(), QLatin1String("wizuser"));
    delete other;
}

/** Test removing multiple users with a LIMIT clause.
 */
void tst_Auth::removeLimit()
{
    loadFixtures();

    // FIXME : remove the first two entries fails
    const QDjangoQuerySet<User> users;
    QCOMPARE(users.limit(0, 2).remove(), false);
    QCOMPARE(users.all().size(), 3);
}

/** Test retrieving a single user.
 */
void tst_Auth::get()
{
    loadFixtures();

    // get an inexistent user
    const QDjangoQuerySet<User> users;
    User *other = users.get(QDjangoWhere("username", QDjangoWhere::Equals, "does_not_exist"));
    QVERIFY(other == 0);

    // get multiple users
    other = users.get(QDjangoWhere());
    QVERIFY(other == 0);

    // get an existing user
    other = users.get(QDjangoWhere("username", QDjangoWhere::Equals, "foouser"));
    QVERIFY(other != 0);
    QCOMPARE(other->username(), QLatin1String("foouser"));
    QCOMPARE(other->password(), QLatin1String("foopass"));
    delete other;
}

/** Test filtering users with a "=" comparison.
 */
void tst_Auth::filter()
{
    loadFixtures();

    // all users
    const QDjangoQuerySet<User> users;
    QDjangoQuerySet<User> qs = users.all();
    CHECKWHERE(qs.where(), QString(), QVariantList());
    QCOMPARE(qs.size(), 3);

    // invalid username
    qs = users.filter(QDjangoWhere("username", QDjangoWhere::Equals, "doesnotexist"));
    CHECKWHERE(qs.where(), QLatin1String("\"user\".\"username\" = ?"), QVariantList() << "doesnotexist");
    QCOMPARE(qs.size(), 0);

    // valid username
    qs = users.filter(QDjangoWhere("username", QDjangoWhere::Equals, "foouser"));
    CHECKWHERE(qs.where(), QLatin1String("\"user\".\"username\" = ?"), QVariantList() << "foouser");
    QCOMPARE(qs.size(), 1);
    User *other = qs.at(0);
    QVERIFY(other != 0);
    QCOMPARE(other->username(), QLatin1String("foouser"));
    QCOMPARE(other->password(), QLatin1String("foopass"));
    delete other;

    // chain filters
    qs = qs.filter(QDjangoWhere("password", QDjangoWhere::Equals, "foopass"));
    CHECKWHERE(qs.where(), QLatin1String("\"user\".\"username\" = ? AND \"user\".\"password\" = ?"), QVariantList() << "foouser" << "foopass");
    QCOMPARE(qs.size(), 1);

    // username in list
    qs = users.filter(QDjangoWhere("username", QDjangoWhere::IsIn, QVariantList() << "foouser" << "wizuser"));
    CHECKWHERE(qs.where(), QLatin1String("\"user\".\"username\" IN (?, ?)"), QVariantList() << "foouser" << "wizuser");
    QCOMPARE(qs.size(), 2);

    // two tests on username
    qs = users.filter(QDjangoWhere("username", QDjangoWhere::Equals, "foouser") ||
                      QDjangoWhere("username", QDjangoWhere::Equals, "baruser"));
    CHECKWHERE(qs.where(), QLatin1String("\"user\".\"username\" = ? OR \"user\".\"username\" = ?"), QVariantList() << "foouser" << "baruser");
    QCOMPARE(qs.size(), 2);
}

/** Test filtering users with a "like" condition.
 */
void tst_Auth::filterLike()
{
    loadFixtures();

    // username starts with "foo"
    const QDjangoQuerySet<User> users;
    QDjangoQuerySet<User> qs = users.filter(QDjangoWhere("username", QDjangoWhere::StartsWith, "foo"));
    QCOMPARE(qs.size(), 1);
    User *other = qs.at(0);
    QVERIFY(other != 0);
    QCOMPARE(other->username(), QLatin1String("foouser"));
    QCOMPARE(other->password(), QLatin1String("foopass"));
    delete other;

    // username ends with "ouser"
    qs = users.filter(QDjangoWhere("username", QDjangoWhere::EndsWith, "ouser"));
    QCOMPARE(qs.size(), 1);
    other = qs.at(0);
    QVERIFY(other != 0);
    QCOMPARE(other->username(), QLatin1String("foouser"));
    QCOMPARE(other->password(), QLatin1String("foopass"));
    delete other;

    // username contains "ou"
    qs = users.filter(QDjangoWhere("username", QDjangoWhere::Contains, "ou"));
    QCOMPARE(qs.size(), 1);
    other = qs.at(0);
    QVERIFY(other != 0);
    QCOMPARE(other->username(), QLatin1String("foouser"));
    QCOMPARE(other->password(), QLatin1String("foopass"));
    delete other;
}

/** Test excluding users with an "=" condition.
 */
void tst_Auth::exclude()
{
    loadFixtures();
    const QDjangoQuerySet<User> users;

    QDjangoQuerySet<User> qs = users.all();
    CHECKWHERE(qs.where(), QString(), QVariantList());
    QCOMPARE(users.all().size(), 3);

    qs = users.exclude(QDjangoWhere("username", QDjangoWhere::Equals, "doesnotexist"));
    CHECKWHERE(qs.where(), QLatin1String("\"user\".\"username\" != ?"), QVariantList() << "doesnotexist");
    QCOMPARE(qs.size(), 3);

    qs = users.exclude(QDjangoWhere("username", QDjangoWhere::Equals, "baruser"));
    CHECKWHERE(qs.where(), QLatin1String("\"user\".\"username\" != ?"), QVariantList() << "baruser");
    QCOMPARE(qs.size(), 2);
    User *other = qs.at(0);
    QVERIFY(other != 0);
    QCOMPARE(other->username(), QLatin1String("foouser"));
    QCOMPARE(other->password(), QLatin1String("foopass"));
    delete other;

    qs = qs.exclude(QDjangoWhere("password", QDjangoWhere::Equals, "barpass"));
    CHECKWHERE(qs.where(), QLatin1String("\"user\".\"username\" != ? AND \"user\".\"password\" != ?"), QVariantList() << "baruser" << "barpass");
    QCOMPARE(qs.size(), 2);
}

/** Test limiting user results.
 */
void tst_Auth::limit()
{
    const QDjangoQuerySet<User> users;

    for (int i = 0; i < 10; i++)
    {
        User user;
        user.setUsername(QString("foouser%1").arg(i));
        user.setPassword(QString("foopass%1").arg(i));
        QCOMPARE(user.save(), true);
    }

    // all results
    QDjangoQuerySet<User> qs = users.limit(0, -1);
    QCOMPARE(qs.size(), 10);

    // all results from offset 1
    qs = users.limit(1, -1);
    QCOMPARE(qs.size(), 9);
    User *other = qs.at(0);
    QCOMPARE(other->username(), QLatin1String("foouser1"));
    delete other;
    other = qs.at(8);
    QCOMPARE(other->username(), QLatin1String("foouser9"));
    delete other;

    // 5 results from offset 0
    qs = users.limit(0, 5);
    QCOMPARE(qs.size(), 5);
    other = qs.at(0);
    QCOMPARE(other->username(), QLatin1String("foouser0"));
    delete other;
    other = qs.at(4);
    QCOMPARE(other->username(), QLatin1String("foouser4"));
    delete other;

    // 6 results from offset 1
    qs = users.limit(1, 8);
    QCOMPARE(qs.size(), 8);
    other = qs.at(0);
    QCOMPARE(other->username(), QLatin1String("foouser1"));
    delete other;
    other = qs.at(7);
    QCOMPARE(other->username(), QLatin1String("foouser8"));
    delete other;
}

/** Test chaining limit statements.
 */
void tst_Auth::subLimit()
{
    const QDjangoQuerySet<User> users;

    for (int i = 0; i < 10; i++)
    {
        User user;
        user.setUsername(QString("foouser%1").arg(i));
        user.setPassword(QString("foopass%1").arg(i));
        QCOMPARE(user.save(), true);
    }

    // base query : 8 results from offset 1
    QDjangoQuerySet<User> refQs = users.limit(1, 8);

    // all sub-results from offset 2
    QDjangoQuerySet<User> qs = refQs.limit(2, -1);
    QCOMPARE(qs.size(), 6);
    User *other = qs.at(0);
    QCOMPARE(other->username(), QLatin1String("foouser3"));
    delete other;
    other = qs.at(5);
    QCOMPARE(other->username(), QLatin1String("foouser8"));
    delete other;

    // 4 sub-results from offset 0
    qs = refQs.limit(0, 4);
    QCOMPARE(qs.size(), 4);
    other = qs.at(0);
    QCOMPARE(other->username(), QLatin1String("foouser1"));
    delete other;
    other = qs.at(3);
    QCOMPARE(other->username(), QLatin1String("foouser4"));
    delete other;

    // 3 sub-results from offset 2
    qs = refQs.limit(2, 3);
    QCOMPARE(qs.size(), 3);
    other = qs.at(0);
    QCOMPARE(other->username(), QLatin1String("foouser3"));
    delete other;
    other = qs.at(2);
    QCOMPARE(other->username(), QLatin1String("foouser5"));
    delete other;
}

/** Test ordering.
 */
void tst_Auth::orderBy()
{
    loadFixtures();

    User user;
    const QDjangoQuerySet<User> users;

    // sort ascending
    QDjangoQuerySet<User> qs = users.orderBy(QStringList() << "username");
    QCOMPARE(qs.count(), 3);
    QCOMPARE(qs.size(), 3);
    QVERIFY(qs.at(0, &user));
    QCOMPARE(user.username(), QLatin1String("baruser"));
    QVERIFY(qs.at(1, &user));
    QCOMPARE(user.username(), QLatin1String("foouser"));
    QVERIFY(qs.at(2, &user));
    QCOMPARE(user.username(), QLatin1String("wizuser"));

    // sort descending
    qs = users.orderBy(QStringList() << "-username");
    QCOMPARE(qs.count(), 3);
    QCOMPARE(qs.size(), 3);
    QVERIFY(qs.at(0, &user));
    QCOMPARE(user.username(), QLatin1String("wizuser"));
    QVERIFY(qs.at(1, &user));
    QCOMPARE(user.username(), QLatin1String("foouser"));
    QVERIFY(qs.at(2, &user));
    QCOMPARE(user.username(), QLatin1String("baruser"));
}

/** Test updating.
 */
void tst_Auth::update()
{
    loadFixtures();

    QVariantMap fields;
    fields.insert("password", "xxx");

    // update no fields
    QCOMPARE(QDjangoQuerySet<User>().update(QVariantMap()), 0);

    // update none
    QCOMPARE(QDjangoQuerySet<User>().none().update(fields), 0);

    // update all
    QDjangoQuerySet<User> qs;
    QCOMPARE(qs.update(fields), 3);

    QDjangoQuerySet<User> all;
    foreach (const User &user, all)
        QCOMPARE(user.password(), QLatin1String("xxx"));

    // update one
    fields.insert("password", "yyy");
    qs = qs.filter(QDjangoWhere("username", QDjangoWhere::Equals, "baruser"));
    QCOMPARE(qs.update(fields), 1);

    all = QDjangoQuerySet<User>();
    foreach (const User &user, all) {
        if (user.username() == "baruser")
            QCOMPARE(user.password(), QLatin1String("yyy"));
        else
            QCOMPARE(user.password(), QLatin1String("xxx"));
    }
}

/** Test retrieving maps of values.
 */
void tst_Auth::values()
{
    loadFixtures();

    const QDjangoQuerySet<User> users;

    // FIXME : test last_login
    QList< QMap<QString, QVariant> > map = users.all().values();
    QCOMPARE(map.size(), 3);
    QCOMPARE(map[0].keys(), QList<QString>() << "date_joined" << "email" << "first_name" << "id" << "is_active" << "is_staff" << "is_superuser" << "last_login" << "last_name" << "password" << "username");
    QCOMPARE(map[0]["username"], QVariant("foouser"));
    QCOMPARE(map[0]["password"], QVariant("foopass"));
    QCOMPARE(map[1].keys(), QList<QString>() << "date_joined" << "email" << "first_name" << "id" << "is_active" << "is_staff" << "is_superuser" << "last_login" << "last_name" << "password" << "username");
    QCOMPARE(map[1]["username"], QVariant("baruser"));
    QCOMPARE(map[1]["password"], QVariant("barpass"));
    QCOMPARE(map[2].keys(), QList<QString>() << "date_joined" << "email" << "first_name" << "id" << "is_active" << "is_staff" << "is_superuser" << "last_login" << "last_name" << "password" << "username");
    QCOMPARE(map[2]["username"], QVariant("wizuser"));
    QCOMPARE(map[2]["password"], QVariant("wizpass"));

    // FIXME : test last_login
    map = users.all().values(QStringList() << "username" << "password");
    QCOMPARE(map.size(), 3);
    QCOMPARE(map[0].keys(), QList<QString>() << "password" << "username");
    QCOMPARE(map[0]["username"], QVariant("foouser"));
    QCOMPARE(map[0]["password"], QVariant("foopass"));
    QCOMPARE(map[1].keys(), QList<QString>() << "password" << "username");
    QCOMPARE(map[1]["username"], QVariant("baruser"));
    QCOMPARE(map[1]["password"], QVariant("barpass"));
    QCOMPARE(map[2].keys(), QList<QString>() << "password" << "username");
    QCOMPARE(map[2]["username"], QVariant("wizuser"));
    QCOMPARE(map[2]["password"], QVariant("wizpass"));
}

/** Test retrieving lists of values.
 */
void tst_Auth::valuesList()
{
    loadFixtures();

    const QDjangoQuerySet<User> users;

    // FIXME : test last_login
    QList< QVariantList > list = users.all().valuesList();
    QCOMPARE(list.size(), 3);
    QCOMPARE(list[0].size(), 11);
    QCOMPARE(list[0][1], QVariant("foouser"));
    QCOMPARE(list[0][5], QVariant("foopass"));
    QCOMPARE(list[1].size(), 11);
    QCOMPARE(list[1][1], QVariant("baruser"));
    QCOMPARE(list[1][5], QVariant("barpass"));
    QCOMPARE(list[2].size(), 11);
    QCOMPARE(list[2][1], QVariant("wizuser"));
    QCOMPARE(list[2][5], QVariant("wizpass"));

    // FIXME : test last_login
    list = users.all().valuesList(QStringList() << "username" << "password");
    QCOMPARE(list.size(), 3);
    QCOMPARE(list[0].size(), 2);
    QCOMPARE(list[0][0], QVariant("foouser"));
    QCOMPARE(list[0][1], QVariant("foopass"));
    QCOMPARE(list[1].size(), 2);
    QCOMPARE(list[1][0], QVariant("baruser"));
    QCOMPARE(list[1][1], QVariant("barpass"));
    QCOMPARE(list[2].size(), 2);
    QCOMPARE(list[2][0], QVariant("wizuser"));
    QCOMPARE(list[2][1], QVariant("wizpass"));
}

void tst_Auth::constIterator()
{
    loadFixtures();
    QVERIFY(!QTest::currentTestFailed());

    const QDjangoQuerySet<User> users = QDjangoQuerySet<User>().orderBy(QStringList("username"));
    const QDjangoQuerySet<User>::ConstIterator first = users.constBegin();
    const QDjangoQuerySet<User>::ConstIterator last = users.constEnd();

    QVERIFY(first != last);
    QVERIFY(first <  last);
    QVERIFY(first <= last);
    QVERIFY(last  >= first);
    QVERIFY(last  >  first);

    QCOMPARE(int(last - first), +3);
    QCOMPARE(int(first - last), -3);

    QDjangoQuerySet<User>::ConstIterator it = first;

    QVERIFY(it != last);
    QVERIFY(it == first);
    QCOMPARE(int(last - it), 3);
    QCOMPARE(int(it - first), 0);

    QCOMPARE(it->username(), QLatin1String("baruser"));
    QCOMPARE((++it)->username(), QLatin1String("foouser"));
    QCOMPARE(it->username(), QLatin1String("foouser"));
    QCOMPARE((it++)->username(), QLatin1String("foouser"));
    QCOMPARE(it->username(), QLatin1String("wizuser"));

    QVERIFY((it - 2) == first);
    QCOMPARE(int(it - first), 2);
    QCOMPARE(int(last - it), 1);

    QVERIFY((it -= 2) == first);
    QCOMPARE(int(it - first), 0);
    QCOMPARE(int(last - it), 3);

    QCOMPARE((*it).username(), QLatin1String("baruser"));
    QCOMPARE((*(it + 2)).username(), QLatin1String("wizuser"));
    QVERIFY(it == first);

    QCOMPARE((*(it += 1)).username(), QLatin1String("foouser"));
    QCOMPARE(int(it - first), 1);

    QTest::ignoreMessage(QtWarningMsg, "QDjangoQuerySet out of bounds");
    QVERIFY(&*(it += 2) == 0);
    QCOMPARE(int(last - it), 0);
    QVERIFY(it == last);

    QCOMPARE((it += -3)->username(), QLatin1String("baruser"));
    QVERIFY(it == first);

    QCOMPARE((it -= -2)->username(), QLatin1String("wizuser"));
    QCOMPARE(int(last - it), 1);

    QCOMPARE((it--)->username(), QLatin1String("wizuser"));
    QCOMPARE(it->username(), QLatin1String("foouser"));
    QCOMPARE(int(last - it), 2);

    QCOMPARE((--it)->username(), QLatin1String("baruser"));
    QCOMPARE(it->username(), QLatin1String("baruser"));
    QCOMPARE(int(last - it), 3);
}


/** Clear database table after each test.
 */
void tst_Auth::cleanup()
{
    QCOMPARE(QDjangoQuerySet<UserGroups>().remove(), true);
    QCOMPARE(QDjangoQuerySet<Message>().remove(), true);
    QCOMPARE(QDjangoQuerySet<Group>().remove(), true);
    QCOMPARE(QDjangoQuerySet<User>().remove(), true);
}

/** Drop database table after running tests.
 */
void tst_Auth::cleanupTestCase()
{
    QVERIFY(QDjango::dropTables());
}

/** Set and get foreign key on a Message object.
 */
void tst_Auth::testRelated()
{
    const QDjangoQuerySet<Message> messages;
    // load fixtures
    QVariant userPk;
    {
        User user;
        user.setUsername("foouser");
        user.setPassword("foopass");
        QCOMPARE(user.save(), true);
        userPk = user.pk();

        Message message;
        message.setUser(&user);
        message.setMessage("test message");
        QCOMPARE(message.save(), true);
    }

    // retrieve message, then user (2 SQL queries)
    Message *uncached = messages.get(
        QDjangoWhere("id", QDjangoWhere::Equals, 1));
    QVERIFY(uncached != 0);
    QCOMPARE(uncached->property("user_id"), userPk);

    // check related user
    User *uncachedUser = uncached->user();
    QVERIFY(uncachedUser != 0);
    QCOMPARE(uncachedUser->pk(), userPk);
    QCOMPARE(uncachedUser->username(), QLatin1String("foouser"));
    QCOMPARE(uncachedUser->password(), QLatin1String("foopass"));
    delete uncached;

    // retrieve message and user (1 SQL query)
    Message *cached = messages.selectRelated().get(
        QDjangoWhere("id", QDjangoWhere::Equals, 1));
    QVERIFY(cached != 0);
    QCOMPARE(cached->property("user_id"), userPk);

    // check related user
    User *cachedUser = cached->user();
    QVERIFY(cachedUser != 0);
    QCOMPARE(cachedUser->pk(), userPk);
    QCOMPARE(cachedUser->username(), QLatin1String("foouser"));
    QCOMPARE(cachedUser->password(), QLatin1String("foopass"));
    delete cached;
}

/** Perform filtering on a foreign field.
 */
void tst_Auth::filterRelated()
{
    const QDjangoQuerySet<Message> messages;
    // load fixtures
    QVariant userPk;
    {
        User user;
        user.setUsername("foouser");
        user.setPassword("foopass");
        QCOMPARE(user.save(), true);
        userPk = user.pk();

        Message message;
        message.setUser(&user);
        message.setMessage("test message");
        QCOMPARE(message.save(), true);
    }

    // perform filtering
    QDjangoQuerySet<Message> qs = messages.filter(
        QDjangoWhere("user__username", QDjangoWhere::Equals, "foouser"));
    CHECKWHERE(qs.where(), QLatin1String("T0.\"username\" = ?"), QVariantList() << "foouser");
    QCOMPARE(qs.size(), 1);

    Message *msg = qs.at(0);
    QVERIFY(msg != 0);
    QCOMPARE(msg->message(), QLatin1String("test message"));
    QCOMPARE(msg->property("user_id"), userPk);
    delete msg;
}

/** Test many-to-many relationships using an intermediate table.
 */
void tst_Auth::testGroups()
{
    const QDjangoQuerySet<UserGroups> userGroups;

    User user;
    user.setUsername("foouser");
    user.setPassword("foopass");
    QCOMPARE(user.save(), true);

    Group group;
    group.setName("foogroup");
    QCOMPARE(group.save(), true);

    UserGroups userGroup;
    userGroup.setUser(&user);
    userGroup.setGroup(&group);
    QCOMPARE(userGroup.save(), true);

    UserGroups *ug = userGroups.selectRelated().get(
        QDjangoWhere("id", QDjangoWhere::Equals, 1));
    QVERIFY(ug != 0);
    QCOMPARE(ug->property("user_id"), user.pk());
    QCOMPARE(ug->property("group_id"), group.pk());
    delete ug;
}

QTEST_MAIN(tst_Auth)
#include "tst_auth.moc"
