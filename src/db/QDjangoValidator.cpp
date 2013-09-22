#include "QDjangoValidator.h"

QDjangoValidator::QDjangoValidator(const QString &message)
    : m_message(message)
{
}

QDjangoValidator::~QDjangoValidator()
{
}

QString QDjangoValidator::message() const
{
    return m_message;
}

void QDjangoValidator::setMessage(const QString &message)
{
    m_message = message;
}

QDjangoRegExpValidator::~QDjangoRegExpValidator()
{
}

QDjangoRegExpValidator::QDjangoRegExpValidator(const QString &regexp,
                                               const QString &message)
    : QDjangoValidator(message),
      m_regularExpression(regexp)
{
}

QDjangoRegExpValidator::QDjangoRegExpValidator(const QRegExp &regexp,
                                               const QString &message)
    : QDjangoValidator(message),
      m_regularExpression(regexp)
{
}

bool QDjangoRegExpValidator::validate(const QVariant &data)
{
    if (!data.canConvert(QVariant::String))
        return false;
    return m_regularExpression.exactMatch(data.toString());
}

