#ifndef QDJANGO_VALIDATOR_H
#define QDJANGO_VALIDATOR_H

#include <QVariant>
#include <QRegExp>

#include "QDjango_p.h"

class QDJANGO_EXPORT QDjangoValidator
{
public:
    virtual ~QDjangoValidator();
    virtual bool validate(const QVariant &data) = 0;

    QString message() const;
    void setMessage(const QString &message);

protected:
    QDjangoValidator(const QString &message);

private:
    QString m_message;

};

class QDJANGO_EXPORT QDjangoRegExpValidator : public QDjangoValidator
{
public:
    explicit QDjangoRegExpValidator(const QString &regexp, const QString &message);
    explicit QDjangoRegExpValidator(const QRegExp &regexp, const QString &message);
    ~QDjangoRegExpValidator();

    virtual bool validate(const QVariant &data);

    QRegExp regularExpression() const;
    void setRegularExpression(const QRegExp &regexp);

private:
    QRegExp m_regularExpression;

};

#endif

