#ifndef MANYTOONE_MODELS_H
#define MANYTOONE_MODELS_H

#include <QDateTime>
#include "QDjangoModel.h"

class Reporter : public QDjangoModel
{
    Q_OBJECT
    Q_PROPERTY(QString first_name READ firstName WRITE setFirstName)
    Q_PROPERTY(QString last_name READ lastName WRITE setLastName)
    Q_PROPERTY(QString email READ email WRITE setEmail)

public:
    Reporter(QObject *parent = 0);

    QString firstName() const;
    void setFirstName(const QString &firstName);

    QString lastName() const;
    void setLastName(const QString &lastName);

    QString email() const;
    void setEmail(const QString &email);

private:
    QString m_firstName;
    QString m_lastName;
    QString m_email;

};

class Article : public QDjangoModel
{
    Q_OBJECT
    Q_PROPERTY(QString headline READ headline WRITE setHeadline)
    Q_PROPERTY(QDateTime pub_date READ pubDate WRITE setPubDate)
    Q_PROPERTY(Reporter* reporter READ reporter WRITE setReporter)

public:
    Article(QObject *parent = 0);

    QString headline() const;
    void setHeadline(const QString &headline);

    QDateTime pubDate() const;
    void setPubDate(const QDateTime &pubDate);

    Reporter *reporter() const;
    void setReporter(Reporter *reporter);

private:
    QString m_headline;
    QDateTime m_pubDate;

};


#endif

