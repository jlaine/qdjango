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

#ifndef QDJANGO_WHERE_P_H
#define QDJANGO_WHERE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QDjango API.
//

#include <QSharedData>

#include "QDjangoWhere.h"

class QDjangoWherePrivate : public QSharedData
{
public:
    static QString operationToString(QDjangoWhere::Operation operation);

    enum Combine
    {
        NoCombine,
        AndCombine,
        OrCombine
    };
    static QString combineToString(Combine combine);

    QDjangoWherePrivate();

    QString key;
    QDjangoWhere::Operation operation;
    QVariant data;

    QList<QDjangoWhere> children;
    Combine combine;
    bool negate;
};

#endif
