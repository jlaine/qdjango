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

#include "auth-models.h"

class tst_FkConstraint : public QDjangoModel
{
    Q_OBJECT
    Q_PROPERTY(User *noConstraint READ noConstraint WRITE setNoConstraint)
    Q_PROPERTY(User *cascadeConstraint READ cascadeConstraint WRITE setCascadeConstraint)
    Q_PROPERTY(User *restrictConstraint READ restrictConstraint WRITE setRestrictConstraint)
    Q_PROPERTY(User *nullConstraint READ nullConstraint WRITE setNullConstraint)
    
    Q_CLASSINFO("cascadeConstraint", "on_delete=cascade")
    Q_CLASSINFO("restrictConstraint", "on_delete=restrict")
    Q_CLASSINFO("nullConstraint", "null=true on_delete=set_null")

public:
    tst_FkConstraint(QObject *parent = 0);

    User *noConstraint() const;
    void setNoConstraint(User *user);

    User *cascadeConstraint() const;
    void setCascadeConstraint(User *user);

    User *restrictConstraint() const;
    void setRestrictConstraint(User *user);

    User *nullConstraint() const;
    void setNullConstraint(User *user);

private slots:
    void testConstraints();
};

