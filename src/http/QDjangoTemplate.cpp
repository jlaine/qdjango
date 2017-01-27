#include "QDJangoTemplate.h"

#include <QFile>
#include <QFileInfo>


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
    QRegExp valRx("\\{\\{ +([a-zA-Z_\\.]+) +\\}\\}");

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

QDJangoTemplate::QDJangoTemplate(const QVariantMap &context) : m_context(context)
{}

bool QDJangoTemplate::load(const QString &fileName)
{
    QFile templ(fileName);
    if (templ.open(QIODevice::ReadOnly)) {
        m_currentDir = QFileInfo(fileName).absoluteDir();
        set(QString::fromUtf8(templ.readAll()));
        return true;
    }
    return false;
}

void QDJangoTemplate::set(const QString &content)
{
    QRegExp tagRx("\\{% +([^%]+) +%\\}");
    int pos = 0;
    int lastPos = 0;
    while ((pos = tagRx.indexIn(content, lastPos)) != -1) {
        if (pos > lastPos)
            m_nodes << qMakePair(false, content.mid(lastPos, pos - lastPos));

        lastPos = pos + tagRx.matchedLength();
        m_nodes << qMakePair(true, tagRx.cap(1));
    }
    m_nodes << qMakePair(false, content.mid(lastPos));
}

void QDJangoTemplate::addToContext(const QString &name, const QVariant &value)
{
    m_context.insert(name, value);
}

void QDJangoTemplate::addToContext(const QVariantMap &context)
{
    foreach (const QString& key, context.keys()) {
        m_context.insert(key, context.value(key));
    }
}

void QDJangoTemplate::setContext(const QVariantMap &context)
{
    m_context = context;
}

void QDJangoTemplate::removeFromContext(const QString &name)
{
    m_context.remove(name);
}

void QDJangoTemplate::clearContext()
{
    m_context.clear();
}

QString QDJangoTemplate::render() const
{
    return render(m_nodes, m_context);
}

QDjangoHttpResponse* QDJangoTemplate::renderToResponse() const
{
    if (m_nodes.isEmpty()) return NULL;

    QDjangoHttpResponse *response = new QDjangoHttpResponse;
    response->setHeader("Content-Type", "text/html; charset=utf-8");
    response->setBody(render().toUtf8());
    return response;
}

QString QDJangoTemplate::renderFile(const QString &name, const QVariantMap &context)
{
    QDJangoTemplate t(context);
    if (!t.load(name)) return QString();
    return t.render();
}

QDjangoHttpResponse *QDJangoTemplate::renderFileToResponse(const QString &fileName, const QVariantMap &context)
{
    QDJangoTemplate t(context);
    if (!t.load(fileName)) return NULL;
    return t.renderToResponse();
}

QString QDJangoTemplate::render(const QList<QDJangoTemplate::Node> nodes, QVariantMap context) const
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
                QFileInfo l_fi(includeRx.cap(1));

                if (l_fi.isAbsolute())
                    output += QDJangoTemplate::renderFile(l_fi.absoluteFilePath(), context);
                else
                    output += QDJangoTemplate::renderFile(m_currentDir.absoluteFilePath(l_fi.filePath()), context);
//                output += QDJangoTemplate::renderFile(":/view/" + includeRx.cap(1), context);
            }
        } else {
            output += substitute(node.second, context);
        }
    }
    return output;
}

int QDJangoTemplate::findBalancing(const QList<Node> nodes, const QString &closeTag, int pos, int *elsePos)
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
