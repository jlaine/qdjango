#include "models.h"

Reporter::Reporter(QObject *parent)
    : QDjangoModel(parent)
{
}

QString Reporter::firstName() const
{
    return m_firstName;
}

void Reporter::setFirstName(const QString &firstName)
{
    m_firstName = firstName;
}

QString Reporter::lastName() const
{
    return m_lastName;
}

void Reporter::setLastName(const QString &lastName)
{
    m_lastName = lastName;
}

QString Reporter::email() const
{
    return m_email;
}

void Reporter::setEmail(const QString &email)
{
    m_email = email;
}

Article::Article(QObject *parent)
    : QDjangoModel(parent)
{
    setForeignKey("reporter", new Reporter(this));
}

QString Article::headline() const
{
    return m_headline;
}

void Article::setHeadline(const QString &headline)
{
    m_headline = headline;
}

QDateTime Article::pubDate() const
{
    return m_pubDate;
}

void Article::setPubDate(const QDateTime &pubDate)
{
    m_pubDate = pubDate;
}

Reporter *Article::reporter() const
{
    return qobject_cast<Reporter*>(foreignKey("reporter"));
}

void Article::setReporter(Reporter *reporter)
{
    setForeignKey("reporter", reporter);
}

