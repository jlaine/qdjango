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

#include <QObject>

/** Tests for the User class.
 */
class TestUser: public QObject
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
    void cleanup();
    void cleanupTestCase();

private:
    void loadFixtures();
};

class TestRelated : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void testGroups();
    void testRelated();
    void filterRelated();
    void testFkConstraint();
    void cleanup();
    void cleanupTestCase();
};

