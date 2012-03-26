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

#include "../../tests/auth-models.h"
#include "http-server.h"

static QVariantMap dump(const QObject *object)
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

template <class T>
class ModelController : public QDjangoHttpController
{
public:
    ModelController();
    QDjangoHttpResponse* respondToRequest(const QDjangoHttpRequest &request);
    void setChangeFields(const QList<QByteArray> fields) {
        m_changeFields = fields;
    }
    void setListFields(const QList<QByteArray> fields) {
        m_listFields = fields;
    }

private:
    QDjangoHttpResponse* changeList(const QDjangoHttpRequest &request);
    QDjangoHttpResponse* changeForm(const QDjangoHttpRequest &request, int objectId);
    QDjangoHttpResponse* deleteForm(const QDjangoHttpRequest &request, int objectId);

    QList<QByteArray> m_changeFields;
    QList<QByteArray> m_listFields;
    QString m_modelName;
    QString m_prefix;
};

template <class T>
ModelController<T>::ModelController()
{
    m_modelName = QString::fromLatin1(T::staticMetaObject.className()).toLower();
    m_prefix = "/" + m_modelName;

    // initialise fields
    const QMetaObject *metaObject = &T::staticMetaObject;
    QVariantList fieldList;
    for (int i = metaObject->propertyOffset(); i < metaObject->propertyCount(); ++i) {
        const char *key = metaObject->property(i).name();
        m_changeFields << key;
    }
}

template <class T>
QDjangoHttpResponse* ModelController<T>::changeForm(const QDjangoHttpRequest &request, int objectId)
{
    T *original = 0;
    if (objectId) {
        original = QDjangoQuerySet<T>().get(QDjangoWhere("pk", QDjangoWhere::Equals, objectId));
        if (!original)
            return serveNotFound(request);
    }

    // collect fields
    QVariantList fieldList;
    foreach (const QByteArray &key, m_changeFields) {
        QVariantMap props;
        props.insert("key", key);
        props.insert("name", QByteArray(key).replace("_", " "));
        if (original)
            props.insert("value", original->property(key));
        fieldList << props;
    }

    if (request.method() == "POST") {
        QUrl url;
        url.setEncodedQuery(request.body());

        if (original) {
            foreach (const QByteArray &key, m_changeFields)
                original->setProperty(key, url.queryItemValue(key));
            original->save();
        } else {
            T obj;
            foreach (const QByteArray &key, m_changeFields)
                obj.setProperty(key, url.queryItemValue(key));
            obj.save();
        }
        return serveRedirect(request, QUrl(m_prefix + "/"));
    } else {
        QVariantMap context;
        context.insert("model_name", m_modelName);
        context.insert("field_list", fieldList);
        if (original) {
            context.insert("original", dump(original));
            context.insert("title", QString("Change %1").arg(m_modelName));
        } else {
            context.insert("title", QString("Add %1").arg(m_modelName));
        }
        return renderToResponse(request, ":/templates/change_form.html", context);
    }
}

template <class T>
QDjangoHttpResponse* ModelController<T>::changeList(const QDjangoHttpRequest &request)
{
    QDjangoQuerySet<T> objects;
    QVariantList objectList;
    foreach (const T &obj, objects)
        objectList << dump(&obj);

    QVariantList fieldList;
    foreach (const QByteArray &key, m_listFields) {
        QVariantMap props;
        props.insert("key", key);
        props.insert("name", QByteArray(key).replace("_", " "));
        fieldList << props;
    }

    QVariantMap context;
    context.insert("title", QString("Select %1 to change").arg(m_modelName));
    context.insert("add_link", QString("Add %1").arg(m_modelName));
    context.insert("model_name", m_modelName);
    context.insert("field_list", fieldList);
    context.insert("object_list", objectList);
    return renderToResponse(request, ":/templates/change_list.html", context);
}

template <class T>
QDjangoHttpResponse* ModelController<T>::deleteForm(const QDjangoHttpRequest &request, int objectId)
{
    T *original = QDjangoQuerySet<T>().get(QDjangoWhere("pk", QDjangoWhere::Equals, objectId));
    if (!original)
        return serveNotFound(request);

    if (request.method() == "POST") {
        original->remove();
        return serveRedirect(request, QUrl(m_prefix + "/"));
    } else {
        QVariantMap context;
        context.insert("model_name", m_modelName);
        context.insert("original", dump(original));
        context.insert("title", "Are you sure?");
        return renderToResponse(request, ":/templates/delete_confirmation.html", context);
    }
}

template <class T>
QDjangoHttpResponse* ModelController<T>::respondToRequest(const QDjangoHttpRequest &request)
{
    QRegExp changeRx("/([0-9]+)/");
    QRegExp deleteRx("/([0-9]+)/delete/");

    const QString path = request.path().mid(m_prefix.size());
    if (path == "/") {
        return changeList(request);
    } else if (path == "/add/") {
        return changeForm(request, 0);
    } else if (changeRx.exactMatch(path)) {
        return changeForm(request, changeRx.cap(1).toInt());
    } else if (deleteRx.exactMatch(path)) {
        return deleteForm(request, deleteRx.cap(1).toInt());
    }
    return serveNotFound(request);
}

class AdminControllerPrivate
{
public:
    ModelController<Group> groupController;
    ModelController<User> userController;
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

    d->userController.setListFields(QList<QByteArray>() << "username" << "email" << "first_name" << "last_name");
    d->groupController.setListFields(QList<QByteArray>() << "name");
}

QDjangoHttpResponse* AdminController::change(const QDjangoHttpRequest &request, const QString &model, const QString &path)
{
    qDebug("change %s", qPrintable(model));
    if (model == "group") {
        return d->groupController.respondToRequest(request);
    } else if (model == "user") {
        return d->userController.respondToRequest(request);
    } else {
        return QDjangoHttpController::serveNotFound(request);
    }
}

QDjangoHttpResponse* AdminController::index(const QDjangoHttpRequest &request)
{
    QVariantMap context;
    context.insert("model_list", QStringList() << "group" << "user");
    context.insert("title", "Administration");
    return renderToResponse(request, ":/templates/index.html", context);
}

QDjangoHttpResponse* AdminController::largeText(const QDjangoHttpRequest &request)
{
    QDjangoHttpResponse *response = new QDjangoHttpResponse;
    response->setHeader("Content-Type", "text/plain");
    const QByteArray line("Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.\n");
    QByteArray data;
    for (int i = 0; i < 1000; ++i) {
        data += line;
    }
    response->setBody(data);
    return response;
}

QDjangoHttpResponse* AdminController::staticFiles(const QDjangoHttpRequest &request, const QString &path)
{
    return QDjangoHttpController::serveStatic(request, ":/base.css");
}

void usage()
{
    fprintf(stderr, "Usage: command [options]\n\n");
    fprintf(stderr, "Available commands:\n");
    fprintf(stderr, "runfcgi\n");
    fprintf(stderr, "runserver\n");
}

void setupUrls(QDjangoUrlResolver *urls, QObject *controller)
{
    urls->addView(QRegExp("^/$"), controller, "index");
    urls->addView(QRegExp("^/large/$"), controller, "largeText");
    urls->addView(QRegExp("^/media/(.+)$"), controller, "staticFiles");
    urls->addView(QRegExp("^/([a-z]+)/(.*)$"), controller, "change");
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
        setupUrls(server->urls(), &controller);
        if (!server->listen(QHostAddress::Any, port)) {
            qWarning("Could not start listening on port %i", port);
            return EXIT_FAILURE;
        }
    } else if (!strcmp(argv[1], "runserver")) {
        QDjangoHttpServer *server = new QDjangoHttpServer;
        setupUrls(server->urls(), &controller);
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

