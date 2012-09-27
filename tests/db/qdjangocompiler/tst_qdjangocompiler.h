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

#include <QObject>

class Object : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString foo READ foo WRITE setFoo)
    Q_PROPERTY(int bar READ bar WRITE setBar)
    Q_PROPERTY(int wiz READ wiz WRITE setWiz)
    Q_PROPERTY(int zoo READ zoo WRITE setZoo)
    Q_PROPERTY(int zzz READ zzz WRITE setZzz)

    Q_CLASSINFO("__meta__", "db_table=foo_table")
    Q_CLASSINFO("foo", "max_length=255")
    Q_CLASSINFO("bar", "db_index=true")
    Q_CLASSINFO("wiz", "ignore_field=true")
    Q_CLASSINFO("zoo", "unique=true")
    Q_CLASSINFO("zzz", "db_column=zzz_column")

public:
    Object(QObject *parent = 0);

    QString foo() const;
    void setFoo(const QString &foo);

    int bar() const;
    void setBar(int bar);

    int wiz() const;
    void setWiz(int wiz);

    int zoo() const;
    void setZoo(int zoo);

    int zzz() const;
    void setZzz(int zzz);

private:
    QString m_foo;
    int m_bar;
    int m_wiz;
    int m_zoo;
    int m_zzz;
};

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

class tst_QDjangoCompiler : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void fieldNames();
    void fieldNamesRecursive();
    void orderLimit();
    void resolve();
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
