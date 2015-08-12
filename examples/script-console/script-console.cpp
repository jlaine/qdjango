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

#include <cstdlib>

#include <QDebug>
#include <QMetaProperty>
#include <QStringList>
#include <QCoreApplication>
#include <QFileInfo>
#include <QScriptEngine>
#include <QSettings>
#include <QSqlDatabase>
#include <QTextStream>
#include <QtPlugin>

#include "QDjango.h"
#include "QDjangoScript.h"

#include "auth-models.h"

static bool wantsToQuit;

Q_DECLARE_METATYPE(QDjangoQuerySet<User>)
Q_DECLARE_METATYPE(QDjangoQuerySet<Group>)
Q_DECLARE_METATYPE(QDjangoQuerySet<Message>)

static QScriptValue qtscript_dir(QScriptContext *ctx, QScriptEngine *eng)
{
    QObject *obj = ctx->argument(0).toQObject();
    if (obj)
    {
        const QMetaObject* meta = obj->metaObject();
        for(int i = meta->propertyOffset(); i < meta->propertyCount(); ++i)
            qDebug() << meta->property(i).name();
    }
    return eng->undefinedValue();
}

static QScriptValue qtscript_load(QScriptContext *ctx, QScriptEngine *eng)
{
    QString name = ctx->argument(0).toString();
    eng->importExtension(name);
    return eng->undefinedValue();
}

static QScriptValue qtscript_quit(QScriptContext *ctx, QScriptEngine *eng)
{
    Q_UNUSED(ctx);
    wantsToQuit = true;
    return eng->undefinedValue();
}

static QScriptValue qtscript_syncdb(QScriptContext *ctx, QScriptEngine *eng)
{
    Q_UNUSED(ctx);
    QDjango::createTables();
    return eng->undefinedValue();
}

static void interactive(QScriptEngine *eng)
{
    QScriptValue global = eng->globalObject();
    if (!global.property(QLatin1String("dir")).isValid())
        global.setProperty(QLatin1String("dir"), eng->newFunction(qtscript_dir));
    if (!global.property(QLatin1String("load")).isValid())
        global.setProperty(QLatin1String("load"), eng->newFunction(qtscript_load));
    if (!global.property(QLatin1String("quit")).isValid())
        global.setProperty(QLatin1String("quit"), eng->newFunction(qtscript_quit));
    if (!global.property(QLatin1String("syncdb")).isValid())
        global.setProperty(QLatin1String("syncdb"), eng->newFunction(qtscript_syncdb));
    wantsToQuit = false;

    QTextStream qin(stdin, QIODevice::ReadOnly);

    const char *qscript_prompt = "qdjango> ";
    const char *dot_prompt = ".... ";
    const char *prompt = qscript_prompt;

    QString code;

    printf("Commands:\n"
           "\tdir(obj) : print the object's properties\n"
           "\tload()   : loads a QtScript extension\n"
           "\tquit()   : exits console\n"
           "\tsyncdb() : creates database tables\n");

    forever {
        QString line;

        printf("%s", prompt);
        fflush(stdout);

        line = qin.readLine();
        if (line.isNull())
            break;

        code += line;
        code += QLatin1Char('\n');

        if (line.trimmed().isEmpty()) {
            continue;

        } else if (! eng->canEvaluate(code)) {
            prompt = dot_prompt;

        } else {
            QScriptValue result = eng->evaluate(code, QLatin1String("typein"));

            code.clear();
            prompt = qscript_prompt;

            if (! result.isUndefined())
                fprintf(stderr, "%s\n", qPrintable(result.toString()));

            if (wantsToQuit)
                break;
        }
    }
}

void usage()
{
    fprintf(stderr, "Usage: qdjango-console [options]\n\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "-d <database>  use <database>\n");
    fprintf(stderr, "-p <plugins>   add <plugins> to plugins search path\n");
}

int main(int argc, char *argv[])
{
    QString databaseName(":memory:");

    /* Create application */
    QCoreApplication app(argc, argv);

    /* Parse command line arguments */
    for (int i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "-h"))
        {
            usage();
            return EXIT_SUCCESS;
        } else if (!strcmp(argv[i], "-d")) {
            if (i == argc - 1 || !strlen(argv[i+1]) || argv[i+1][0] == '-')
            {
                fprintf(stderr, "Option -d requires an argument\n");
                usage();
                return EXIT_FAILURE;
            }
            databaseName = QString::fromLocal8Bit(argv[++i]);
        } else if (!strcmp(argv[i], "-p")) {
            if (i == argc - 1 || !strlen(argv[i+1]) || argv[i+1][0] == '-')
            {
                fprintf(stderr, "Option -p requires an argument\n");
                usage();
                return EXIT_FAILURE;
            }
            app.setLibraryPaths(app.libraryPaths() << QString::fromLocal8Bit(argv[++i]));
        }
    }

    /* Open database */
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(databaseName);
    if (!db.open())
    {
        fprintf(stderr, "Could not access database '%s'\n", databaseName.toLocal8Bit().constData());
        return EXIT_FAILURE;
    }
    QDjango::setDatabase(db);

    /* Run interactive shell */ 
    QScriptEngine *engine = new QScriptEngine();
    QDjangoScript::registerWhere(engine);
    QDjangoScript::registerModel<User>(engine);
    QDjangoScript::registerModel<Group>(engine);
    QDjangoScript::registerModel<Message>(engine);

    qDebug() << "Available extensions: " << engine->availableExtensions();
    interactive(engine);

    return EXIT_SUCCESS;
}

