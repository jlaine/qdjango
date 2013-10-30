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
      m_regularExpression(regexp),
      m_matchCounter(0)
{
}

QDjangoRegExpValidator::QDjangoRegExpValidator(const QRegExp &regexp,
                                               const QString &message)
    : QDjangoValidator(message),
      m_regularExpression(regexp),
      m_matchCounter(0)
{
}

bool QDjangoRegExpValidator::validate(const QVariant &data)
{
    if (!data.canConvert(QVariant::String))
        return false;

    // TODO: migrate to QRegularExpression in Qt5
    // due to a limitation of QRegExp, it seems that if you use the same
    // expression over and over, it keeps results in an internal map
    // so we opt for this less-than-ideal solution..
    if (m_matchCounter >= 500) {
        m_regularExpression = QRegExp(m_regularExpression);
        m_matchCounter = 0;
    }

    m_matchCounter++;
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

