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

QDjangoUrlValidator::QDjangoUrlValidator(const QString &message)
    : QDjangoRegExpValidator(QRegExp(
          "^(?:http|ftp)s?://" \
          "(?:(?:[A-Z0-9](?:[A-Z0-9-]{0,61}[A-Z0-9])?\\.)+(?:[A-Z]{2,6}\\.?|[A-Z0-9-]{2,}\\.?)|" \
          "localhost|" \
          "\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}|" \
          "\\[?[A-F0-9]*:[A-F0-9:]+\\]?)" \
          "(?::\\d+)?" \
          "(?:/?|[/?]\\S+)$", Qt::CaseInsensitive), message)
{
}

QDjangoUrlValidator::~QDjangoUrlValidator()
{
}

QDjangoIPv4Validator::QDjangoIPv4Validator(const QString &message)
    : QDjangoRegExpValidator("^(25[0-5]|2[0-4]\\d|[0-1]?\\d?\\d)(\\.(25[0-5]|2[0-4]\\d|[0-1]?\\d?\\d)){3}$", message)
{
}

QDjangoIPv4Validator::~QDjangoIPv4Validator()
{
}

