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

#ifndef QDJANGO_SCRIPT_P_H
#define QDJANGO_SCRIPT_P_H

#if defined(QDJANGO_SHARED)
#  if defined(QDJANGO_SCRIPT_BUILD)
#    define QDJANGO_SCRIPT_EXPORT Q_DECL_EXPORT
#  else
#    define QDJANGO_SCRIPT_EXPORT Q_DECL_IMPORT
#  endif
#else
#  define QDJANGO_SCRIPT_EXPORT
#endif

#include "QDjangoWhere.h"
//
//  W A R N I N G
//  -------------
//
// This file is not part of the QDjango API.
//

QDJANGO_SCRIPT_EXPORT QDjangoWhere QDjangoWhereFromScriptValue(QScriptEngine *engine, const QScriptValue &obj);

template <class T>
static QScriptValue QDjangoQuerySet_all(QScriptContext *context, QScriptEngine *engine)
{
    QDjangoQuerySet<T> qs = engine->fromScriptValue< QDjangoQuerySet<T> >(context->thisObject());
    return engine->toScriptValue(qs.all());
}

template <class T>
static QScriptValue QDjangoQuerySet_at(QScriptContext *context, QScriptEngine *engine)
{
    QDjangoQuerySet<T> qs = engine->fromScriptValue< QDjangoQuerySet<T> >(context->thisObject());
    //QDjangoQuerySet<T> qs = context->thisObject().toVariant().value< QDjangoQuerySet<T> >();
    int index = context->argument(0).toInteger();
    return engine->newQObject(qs.at(index), QScriptEngine::ScriptOwnership);
}

template <class T>
static QScriptValue QDjangoQuerySet_count(QScriptContext *context, QScriptEngine *engine)
{
    QDjangoQuerySet<T> qs = engine->fromScriptValue< QDjangoQuerySet<T> >(context->thisObject());
    return QScriptValue(engine, qs.count());
}

template <class T>
static QScriptValue QDjangoQuerySet_exclude(QScriptContext *context, QScriptEngine *engine)
{
    QDjangoQuerySet<T> qs = engine->fromScriptValue< QDjangoQuerySet<T> >(context->thisObject());
    QDjangoWhere where = QDjangoWhereFromScriptValue(engine, context->argument(0));
    return engine->toScriptValue(qs.exclude(where));
}

template <class T>
static QScriptValue QDjangoQuerySet_filter(QScriptContext *context, QScriptEngine *engine)
{
    QDjangoQuerySet<T> qs = engine->fromScriptValue< QDjangoQuerySet<T> >(context->thisObject());
    QDjangoWhere where = QDjangoWhereFromScriptValue(engine, context->argument(0));
    return engine->toScriptValue(qs.filter(where));
}

template <class T>
static QScriptValue QDjangoQuerySet_get(QScriptContext *context, QScriptEngine *engine)
{
    QDjangoQuerySet<T> qs = engine->fromScriptValue< QDjangoQuerySet<T> >(context->thisObject());
    QDjangoWhere where = QDjangoWhereFromScriptValue(engine, context->argument(0));
    return engine->newQObject(qs.get(where), QScriptEngine::ScriptOwnership);
}

template <class T>
static QScriptValue QDjangoQuerySet_limit(QScriptContext *context, QScriptEngine *engine)
{
    QDjangoQuerySet<T> qs = engine->fromScriptValue< QDjangoQuerySet<T> >(context->thisObject());
    const int pos = context->argument(0).toInteger();
    const int limit = (context->argumentCount() > 1) ? context->argument(1).toInteger() : 1;
    return engine->toScriptValue(qs.limit(pos, limit));
}

template <class T>
static QScriptValue QDjangoQuerySet_remove(QScriptContext *context, QScriptEngine *engine)
{
    QDjangoQuerySet<T> qs = engine->fromScriptValue< QDjangoQuerySet<T> >(context->thisObject());
    return QScriptValue(engine, qs.remove());
}

template <class T>
static QScriptValue QDjangoQuerySet_size(QScriptContext *context, QScriptEngine *engine)
{
    QDjangoQuerySet<T> qs = engine->fromScriptValue< QDjangoQuerySet<T> >(context->thisObject());
    return QScriptValue(engine, qs.size());
}

template <class T>
static QScriptValue QDjangoQuerySet_toString(QScriptContext *context, QScriptEngine *engine)
{
    QDjangoQuerySet<T> qs = engine->fromScriptValue< QDjangoQuerySet<T> >(context->thisObject());
    return QScriptValue(engine, QString("QuerySet<%1>(%2)").arg(T::staticMetaObject.className(), qs.where().sql(QDjango::database())));
}

template <class T>
static QScriptValue QDjangoModel_new(QScriptContext *context, QScriptEngine *engine)
{
    Q_UNUSED(context);
    return engine->newQObject(new T, QScriptEngine::ScriptOwnership);
}

#endif
