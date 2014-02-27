#ifndef QDJANGOTEMPLATE_H
#define QDJANGOTEMPLATE_H

#include <QDjangoHttpResponse.h>
#include "QDjangoHttpRequest.h"

#include <QDir>
#include <QString>
#include <QVariantMap>

class QDJangoTemplate
{
private:
    typedef QPair<bool, QString> Node;

public:
    explicit QDJangoTemplate(const QVariantMap& context = QVariantMap());

    bool load(const QString& fileName);
    void set(const QString& content);

    void addToContext(const QString& name, const QVariant& value);
    void addToContext(const QVariantMap& context);
    void setContext(const QVariantMap& context);
    void removeFromContext(const QString& name);
    void clearContext();

    QString render() const;
    QDjangoHttpResponse *renderToResponse() const;

    static QString renderFile(const QString& fileName, const QVariantMap &context = QVariantMap());
    static QDjangoHttpResponse* renderFileToResponse(const QString& fileName, const QVariantMap &context = QVariantMap());

private:
    QString render(const QList<Node> nodes, QVariantMap m_context) const;
    static int findBalancing(const QList<Node> nodes, const QString &closeTag, int pos, int *elsePos = 0);

private:
    QList<Node> m_nodes;
    QVariantMap m_context;
    QDir m_currentDir;
};

#endif // QDJANGOTEMPLATE_H
