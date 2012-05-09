/*
 * Copyright (C) 2010-2012 Jeremy Lainé
 * Contact: http://code.google.com/p/qdjango/
 *
 * This file is part of the QDjango Library.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
    enum Combine
    {
        NoCombine,
        AndCombine,
        OrCombine,
    };

    QDjangoWherePrivate();

    QString key;
    QDjangoWhere::Operation operation;
    QVariant data;

    QList<QDjangoWhere> children;
    Combine combine;
    bool negate;
};

#endif
