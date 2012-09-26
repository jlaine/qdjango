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

#include <QtTest/QtTest>

#include "foreignkey.h"
#include "main.h"

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

/** Test foreign key constraint sql generation
 */
void tst_FkConstraint::testConstraints()
{
  QDjangoMetaModel tst_ = QDjango::registerModel<tst_FkConstraint>();
  QCOMPARE(tst_.createTableSql(), QStringList()
        << QLatin1String("CREATE TABLE \"tst_fkconstraint\" ("
            "\"id\" integer NOT NULL PRIMARY KEY AUTOINCREMENT, "
            "\"noConstraint_id\" integer NOT NULL REFERENCES \"user\" (\"id\"), "
            "\"cascadeConstraint_id\" integer NOT NULL REFERENCES \"user\" (\"id\") ON DELETE CASCADE, "
            "\"restrictConstraint_id\" integer NOT NULL REFERENCES \"user\" (\"id\") ON DELETE RESTRICT, "
            "\"nullConstraint_id\" integer REFERENCES \"user\" (\"id\") ON DELETE SET NULL"
            ")")
        << QLatin1String("CREATE INDEX \"tst_fkconstraint_f388fc3c\" ON \"tst_fkconstraint\" (\"noConstraint_id\")")
        << QLatin1String("CREATE INDEX \"tst_fkconstraint_4634d592\" ON \"tst_fkconstraint\" (\"cascadeConstraint_id\")")
        << QLatin1String("CREATE INDEX \"tst_fkconstraint_728cefe1\" ON \"tst_fkconstraint\" (\"restrictConstraint_id\")")
        << QLatin1String("CREATE INDEX \"tst_fkconstraint_44c71620\" ON \"tst_fkconstraint\" (\"nullConstraint_id\")"));
}

