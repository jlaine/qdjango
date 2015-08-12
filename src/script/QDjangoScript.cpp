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

#include <QDebug>
#include <QScriptEngine>
#include <QScriptValue>
#include <QScriptValueIterator>

#include "QDjangoScript.h"
#include "QDjangoWhere.h"

QDjangoWhere QDjangoWhereFromScriptValue(QScriptEngine *engine, const QScriptValue &obj)
{
    if (obj.prototype().equals(engine->defaultPrototype(qMetaTypeId<QDjangoWhere>()))) {
        return engine->fromScriptValue<QDjangoWhere>(obj);
    }

    QDjangoWhere where;
    QScriptValueIterator it(obj);
    while (it.hasNext()) {
        it.next();
        QString key = it.name();
        QDjangoWhere::Operation op = QDjangoWhere::Equals;
        if (key.endsWith(QLatin1String("__lt"))) {
            key.chop(4);
            op = QDjangoWhere::LessThan;
        }
        else if (key.endsWith(QLatin1String("__lte"))) {
            key.chop(5);
            op = QDjangoWhere::LessOrEquals;
        }
        else if (key.endsWith(QLatin1String("__gt"))) {
            key.chop(4);
            op = QDjangoWhere::GreaterThan;
        }
        else if (key.endsWith(QLatin1String("__gte"))) {
            key.chop(5);
            op = QDjangoWhere::GreaterOrEquals;
        }
        else if (key.endsWith(QLatin1String("__startswith"))) {
            key.chop(12);
            op = QDjangoWhere::StartsWith;
        }
        else if (key.endsWith(QLatin1String("__endswith"))) {
            key.chop(10);
            op = QDjangoWhere::EndsWith;
        }
        else if (key.endsWith(QLatin1String("__contains"))) {
            key.chop(10);
            op = QDjangoWhere::Contains;
        }
        else if (key.endsWith(QLatin1String("__in"))) {
            key.chop(4);
            op = QDjangoWhere::IsIn;
        }
        where = where && QDjangoWhere(key, op, it.value().toVariant());
    }
    return where;
} 

static QScriptValue newWhere(QScriptContext *context, QScriptEngine *engine)
{
    QDjangoWhere where;
    if (context->argumentCount() == 1 && context->argument(0).isObject()) {
        where = QDjangoWhereFromScriptValue(engine, context->argument(0));
    }
    return engine->toScriptValue(where);
}

static QScriptValue whereAnd(QScriptContext *context, QScriptEngine *engine)
{
    QDjangoWhere q = engine->fromScriptValue<QDjangoWhere>(context->thisObject());
    QDjangoWhere other = QDjangoWhereFromScriptValue(engine, context->argument(0));
    return engine->toScriptValue(q && other);
}

static QScriptValue whereOr(QScriptContext *context, QScriptEngine *engine)
{
    QDjangoWhere q = engine->fromScriptValue<QDjangoWhere>(context->thisObject());
    QDjangoWhere other = QDjangoWhereFromScriptValue(engine, context->argument(0));
    return engine->toScriptValue(q || other);
}

static QScriptValue whereToString(QScriptContext *context, QScriptEngine *engine)
{
    QDjangoWhere q = engine->fromScriptValue<QDjangoWhere>(context->thisObject());
    return engine->toScriptValue(QLatin1String("Q(") + q.sql(QDjango::database()) + QLatin1String(")"));
}

/** Makes the QDjangoWhere class available to the given QScriptEngine.
 *
 * \param engine
 */
void QDjangoScript::registerWhere(QScriptEngine *engine)
{
    QScriptValue whereProto = engine->newObject();
    whereProto.setProperty(QLatin1String("and"), engine->newFunction(whereAnd));
    whereProto.setProperty(QLatin1String("or"), engine->newFunction(whereOr));
    whereProto.setProperty(QLatin1String("toString"), engine->newFunction(whereToString));
    engine->setDefaultPrototype(qMetaTypeId<QDjangoWhere>(), whereProto);

    QScriptValue ctor = engine->newFunction(newWhere);
    engine->globalObject().setProperty(QLatin1String("Q"), ctor, QScriptValue::ReadOnly);
}

