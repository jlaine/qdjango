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
