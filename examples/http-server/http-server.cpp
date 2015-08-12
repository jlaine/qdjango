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

#include <QCoreApplication>
#include <QFile>
#include <QMetaObject>
#include <QMetaProperty>
#include <QUrl>

#include "QDjango.h"
#include "QDjangoQuerySet.h"
#include "QDjangoFastCgiServer.h"
#include "QDjangoHttpController.h"
#include "QDjangoHttpRequest.h"
#include "QDjangoHttpResponse.h"
#include "QDjangoHttpServer.h"
#include "QDjangoUrlResolver.h"

#include "auth-models.h"
#include "http-server.h"

class ModelAdminFetcher
{
public:
    virtual QDjangoModel *createObject() const = 0;
    virtual QVariantMap dumpObject(const QObject *object) const = 0;
    virtual QDjangoModel *getObject(const QString& objectId) const = 0;
    virtual QVariantList listObjects(const QList<QByteArray> &listFields) const = 0;
    virtual QString modelName() const = 0;
};

template<class T>
class ModelAdminFetcherImpl : public ModelAdminFetcher
{
public:
    QDjangoModel *createObject() const
    {
        return new T;
    }

    QVariantMap dumpObject(const QObject *object) const
    {
        const QMetaObject *metaObject = object->metaObject();
        QVariantMap props;
        props.insert("pk", object->property("pk"));
        for (int i = metaObject->propertyOffset(); i < metaObject->propertyCount(); ++i) {
            const char *key = metaObject->property(i).name();
            props.insert(key, object->property(key));
        }
        return props;
    }

    QDjangoModel *getObject(const QString& objectId) const
    {
        return QDjangoQuerySet<T>().get(QDjangoWhere("pk", QDjangoWhere::Equals, objectId));
    }

    QVariantList listObjects(const QList<QByteArray> &listFields) const
    {
        QVariantList objectList;
        QDjangoQuerySet<T> objects;
        foreach (const T &obj, objects) {
            QVariantMap dump = dumpObject(&obj);

            // get ordered list of properties to show
            QVariantList propList;
            foreach (const QByteArray &key, listFields) {
                propList << dump.value(key);
            }
            dump.insert("value_list", propList);

            objectList << dump;
        }
        return objectList;
    }

    QString modelName() const
    {
        return QString::fromLatin1(T::staticMetaObject.className()).toLower();
    }
};

static QVariant evaluate(const QString &input, const QVariantMap &context)
{
    const QStringList bits = input.split(".");
    QVariant value = context;
    foreach (const QString &bit, bits) {
        value = value.toMap().value(bit);
    }
    //qDebug("evaluate(%s): %s", qPrintable(input), qPrintable(value.toString()));
    return value;
}

static QString substitute(const QString &input, const QVariantMap &context)
{
    QRegExp valRx("\\{\\{ +([a-z_\\.]+) +\\}\\}");

    QString output;
    int pos = 0;
    int lastPos = 0;
    while ((pos = valRx.indexIn(input, lastPos)) != -1) {
        output += input.mid(lastPos, pos - lastPos);
        lastPos = pos + valRx.matchedLength();
        output += evaluate(valRx.cap(1), context).toString();
    }
    output += input.mid(lastPos);
    return output;
}

typedef QPair<bool, QString> Node;

static QList<Node> tokenize(const QString &input)
{
    QList<Node> output;
    QRegExp tagRx("\\{% +([^%]+) +%\\}");
    int pos = 0;
    int lastPos = 0;
    while ((pos = tagRx.indexIn(input, lastPos)) != -1) {
        if (pos > lastPos)
            output << qMakePair(false, input.mid(lastPos, pos - lastPos));

        lastPos = pos + tagRx.matchedLength();
        output << qMakePair(true, tagRx.cap(1));
    }
    output << qMakePair(false, input.mid(lastPos));
    return output;
}

static int findBalancing(const QList<Node> nodes, const QString &closeTag, int pos, int *elsePos = 0)
{
    const QString openTag = nodes[pos].second.split(" ").first();
    int level = 0;
    if (elsePos)
        *elsePos = -1;
    for (pos = pos + 1; pos < nodes.size(); ++pos) {
        if (nodes[pos].first && nodes[pos].second.startsWith(openTag)) {
            level++;
        }
        else if (nodes[pos].first && nodes[pos].second == closeTag) {
            if (!level)
                return pos;
            level--;
        } else if (!level && nodes[pos].second == "else") {
            if (elsePos)
                *elsePos = pos;
        }
    }
    qWarning("Could not find %s tag", qPrintable(closeTag));
    return -1;
}

QString renderTemplate(const QString &name, const QVariantMap &context);

static QString render(const QList<Node> &nodes, const QVariantMap &context)
{
    QRegExp forRx("for ([a-z_]+) in ([a-z_\\.]+)");
    QRegExp includeRx("include \"([^\"]+)\"");

    QString output;
    for (int i = 0; i < nodes.size(); ++i) {
        const Node &node = nodes[i];
        if (node.first) {
            //qDebug("Processing tag %s", qPrintable(node.second));
            QStringList tagArgs = node.second.split(" ");
            const QString tagName = tagArgs.takeFirst();
            if (node.second == "comment") {
                const int endPos = findBalancing(nodes, "endcomment", i++);
                if (endPos < 0)
                    return output;
                i = endPos;
            } else if (forRx.exactMatch(node.second)) {
                const int endPos = findBalancing(nodes, "endfor", i++);
                if (endPos < 0)
                    return output;

                const QVariantList list = evaluate(forRx.cap(2), context).toList();
                QVariantMap forLoop;
                int counter0 = 0;
                foreach (const QVariant &val, list) {
                    forLoop.insert("counter", counter0 + 1);
                    forLoop.insert("counter0", counter0);
                    if (!counter0)
                        forLoop.insert("first", true);

                    QVariantMap subContext = context;
                    subContext.insert(forRx.cap(1), val);
                    subContext.insert("forloop", forLoop);
                    output += render(nodes.mid(i, endPos - i), subContext);
                    counter0++;
                }
                i = endPos;
            } else if (tagName == "if") {
                int elsePos = -1;
                const int endPos = findBalancing(nodes, "endif", i++, &elsePos);
                if (endPos < 0)
                    return output;

                bool isTrue = false;
                QRegExp ifRx("if ([a-z_\\.]+) (!=|==) \"([^\"]*)\"");
                if (ifRx.exactMatch(node.second)) {
                    const QVariant value = evaluate(ifRx.cap(1), context);
                    const QString op = ifRx.cap(2);
                    const QString opValue = ifRx.cap(3);
                    if ((op == "==" && value.toString() == opValue) ||
                        (op == "!=" && value.toString() != opValue)) {
                        isTrue = true;
                    }
                } else if (tagArgs.size() == 1) {
                    const QVariant value = evaluate(tagArgs[0], context);
                    if (value.toList().size() || value.toMap().size() || value.toString().size())
                        isTrue = true;
                }
                if (isTrue) {
                    output += render(nodes.mid(i, (elsePos > 0 ? elsePos : endPos) - i), context);
                } else if (elsePos > 0) {
                    output += render(nodes.mid(elsePos, endPos - elsePos), context);
                }
                i = endPos;
            } else if (includeRx.exactMatch(node.second)) {
                output += renderTemplate(":/templates/" + includeRx.cap(1), context);
            }
        } else {
            output += substitute(node.second, context);
        }
    }
    return output;
}

QString renderTemplate(const QString &name, const QVariantMap &context)
{
    QFile templ(name);
    if (templ.open(QIODevice::ReadOnly)) {
        const QString data = QString::fromUtf8(templ.readAll());
        return render(tokenize(data), context);
    }
    return QString();
}

static QDjangoHttpResponse *renderToResponse(const QDjangoHttpRequest &request, const QString &name, const QVariantMap &context)
{
    Q_UNUSED(request);
    QDjangoHttpResponse *response = new QDjangoHttpResponse;
    response->setHeader("Content-Type", "text/html; charset=utf-8");
    response->setBody(renderTemplate(name, context).toUtf8());
    return response;
}

class ModelAdminPrivate
{
public:
    QDjangoHttpResponse* redirectHome(const QDjangoHttpRequest &request)
    {
        return QDjangoHttpController::serveRedirect(request, QUrl("/" + modelFetcher->modelName() + "/"));
    }

    QList<QByteArray> changeFields;
    QList<QByteArray> listFields;
    ModelAdminFetcher *modelFetcher;
    QDjangoUrlResolver *urlResolver;
};

ModelAdmin::ModelAdmin(ModelAdminFetcher *fetcher, QObject *parent)
    : QObject(parent)
{
    d = new ModelAdminPrivate;
    d->modelFetcher = fetcher;
    d->urlResolver = new QDjangoUrlResolver(this);
    d->urlResolver->set(QRegExp("^$"), this, "changeList");
    d->urlResolver->set(QRegExp("^add/$"), this, "addForm");
    d->urlResolver->set(QRegExp("^([0-9]+)/"), this, "changeForm");
    d->urlResolver->set(QRegExp("^([0-9]+)/delete/"), this, "deleteForm");
}

ModelAdmin::~ModelAdmin()
{
    delete d;
}

QList<QByteArray> ModelAdmin::changeFields() const
{
    return d->changeFields;
}

void ModelAdmin::setChangeFields(const QList<QByteArray> fields)
{
    d->changeFields = fields;
}

QList<QByteArray> ModelAdmin::listFields() const
{
    return d->listFields;
}

void ModelAdmin::setListFields(const QList<QByteArray> fields)
{
    d->listFields = fields;
}

QDjangoHttpResponse* ModelAdmin::addForm(const QDjangoHttpRequest &request)
{
    const QString modelName = d->modelFetcher->modelName();

    // collect fields
    QVariantList fieldList;
    foreach (const QByteArray &key, d->changeFields) {
        QVariantMap props;
        props.insert("key", key);
        props.insert("name", QByteArray(key).replace("_", " "));
        fieldList << props;
    }

    if (request.method() == "POST") {
        QDjangoModel *obj = d->modelFetcher->createObject();
        foreach (const QByteArray &key, d->changeFields)
            obj->setProperty(key, request.post(key));
        obj->save();
        delete obj;
        return d->redirectHome(request);
    } else {
        QVariantMap context;
        context.insert("model_name", modelName);
        context.insert("field_list", fieldList);
        context.insert("title", QString("Add %1").arg(modelName));
        return renderToResponse(request, ":/templates/change_form.html", context);
    }
}

QDjangoHttpResponse* ModelAdmin::changeForm(const QDjangoHttpRequest &request, const QString &objectId)
{
    QDjangoModel *original = d->modelFetcher->getObject(objectId);
    if (!original)
        return QDjangoHttpController::serveNotFound(request);

    // collect fields
    QVariantList fieldList;
    foreach (const QByteArray &key, d->changeFields) {
        QVariantMap props;
        props.insert("key", key);
        props.insert("name", QByteArray(key).replace("_", " "));
        props.insert("value", original->property(key));
        fieldList << props;
    }

    if (request.method() == "POST") {
        foreach (const QByteArray &key, d->changeFields)
            original->setProperty(key, request.post(key));
        original->save();
        return d->redirectHome(request);
    } else {
        const QString modelName = d->modelFetcher->modelName();
        QVariantMap context;
        context.insert("model_name", modelName);
        context.insert("field_list", fieldList);
        context.insert("original", d->modelFetcher->dumpObject(original));
        context.insert("title", QString("Change %1").arg(modelName));
        return renderToResponse(request, ":/templates/change_form.html", context);
    }
}

QDjangoHttpResponse* ModelAdmin::changeList(const QDjangoHttpRequest &request)
{
    QVariantList objectList = d->modelFetcher->listObjects(d->listFields);

    QVariantList fieldList;
    foreach (const QByteArray &key, d->listFields) {
        QVariantMap props;
        props.insert("key", key);
        props.insert("name", QByteArray(key).replace("_", " "));
        fieldList << props;
    }

    const QString modelName = d->modelFetcher->modelName();
    QVariantMap context;
    context.insert("title", QString("Select %1 to change").arg(modelName));
    context.insert("add_link", QString("Add %1").arg(modelName));
    context.insert("edit_link", QString("Edit"));
    context.insert("delete_link", QString("Remove"));
    context.insert("model_name", modelName);
    context.insert("field_list", fieldList);
    context.insert("object_list", objectList);
    return renderToResponse(request, ":/templates/change_list.html", context);
}

QDjangoHttpResponse* ModelAdmin::deleteForm(const QDjangoHttpRequest &request, const QString &objectId)
{
    QDjangoModel *original = d->modelFetcher->getObject(objectId);
    if (!original)
        return QDjangoHttpController::serveNotFound(request);

    if (request.method() == "POST") {
        original->remove();
        return d->redirectHome(request);
    } else {
        const QString modelName = d->modelFetcher->modelName();
        QVariantMap context;
        context.insert("model_name", modelName);
        context.insert("original", d->modelFetcher->dumpObject(original));
        context.insert("title", "Are you sure?");
        return renderToResponse(request, ":/templates/delete_confirmation.html", context);
    }
}

QDjangoUrlResolver *ModelAdmin::urls() const
{
    return d->urlResolver;
}

class AdminControllerPrivate
{
public:
};

AdminController::AdminController(QObject *parent)
    : QObject(parent)
    , d(new AdminControllerPrivate)
{
    const QString databaseName("test.db");

    /* Open database */
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(databaseName);
    if (!db.open()) {
        qWarning("Could not access database '%s'\n", qPrintable(databaseName));
        return;
    }
    QDjango::setDatabase(db);
    QDjango::registerModel<Group>();
    QDjango::registerModel<User>();
    QDjango::createTables();
}

QDjangoHttpResponse* AdminController::index(const QDjangoHttpRequest &request)
{
    QVariantMap context;
    context.insert("model_list", QStringList() << "group" << "user");
    context.insert("title", "Administration");
    return renderToResponse(request, ":/templates/index.html", context);
}

QDjangoHttpResponse* AdminController::staticFiles(const QDjangoHttpRequest &request, const QString &path)
{
    Q_UNUSED(path);

    return QDjangoHttpController::serveStatic(request, ":/base.css");
}

void usage()
{
    fprintf(stderr, "Usage: command [options]\n\n");
    fprintf(stderr, "Available commands:\n");
    fprintf(stderr, "runfcgi\n");
    fprintf(stderr, "runserver\n");
}

void AdminController::setupUrls(QDjangoUrlResolver *urls)
{
    urls->set(QRegExp("^$"), this, "index");
    urls->set(QRegExp("^static/admin/(.+)$"), this, "staticFiles");

    ModelAdmin *groupAdmin = new ModelAdmin(new ModelAdminFetcherImpl<Group>);
    groupAdmin->setChangeFields(QList<QByteArray>() << "name");
    groupAdmin->setListFields(QList<QByteArray>() << "name");
    urls->include(QRegExp("^group/"), groupAdmin->urls());

    ModelAdmin *userAdmin = new ModelAdmin(new ModelAdminFetcherImpl<User>);
    userAdmin->setChangeFields(QList<QByteArray>() << "username" << "email" << "first_name" << "last_name");
    userAdmin->setListFields(QList<QByteArray>() << "username" << "email" << "first_name" << "last_name");
    urls->include(QRegExp("^user/"), userAdmin->urls());
}

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    AdminController controller;

    const quint16 port = 8000;

    if (argc < 2) {
        usage();
        return EXIT_FAILURE;
    }

    QDjangoUrlResolver urls;

    if (!strcmp(argv[1], "runfcgi")) {
        QDjangoFastCgiServer *server = new QDjangoFastCgiServer;
        controller.setupUrls(server->urls());
        if (!server->listen(QHostAddress::Any, port)) {
            qWarning("Could not start listening on port %i", port);
            return EXIT_FAILURE;
        }
    } else if (!strcmp(argv[1], "runserver")) {
        QDjangoHttpServer *server = new QDjangoHttpServer;
        controller.setupUrls(server->urls());
        if (!server->listen(QHostAddress::Any, port)) {
            qWarning("Could not start listening on port %i", port);
            return EXIT_FAILURE;
        }
    } else {
        usage();
        return EXIT_FAILURE;
    }

    return app.exec();
}

