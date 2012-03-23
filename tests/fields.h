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

#include <QDate>

#include "QDjangoModel.h"

class tst_Bool : public QDjangoModel
{
    Q_OBJECT
    Q_PROPERTY(bool value READ value WRITE setValue)

public:
    bool value() const { return m_value; }
    void setValue(bool value) { m_value = value; }

private slots:
    void testValue();

private:
    bool m_value;
};

class tst_ByteArray : public QDjangoModel
{
    Q_OBJECT
    Q_PROPERTY(QByteArray value READ value WRITE setValue)

public:
    QByteArray value() const { return m_value; }
    void setValue(const QByteArray &value) { m_value = value; }

private slots:
    void testValue();

private:
    QByteArray m_value;
};

class tst_Date : public QDjangoModel
{
    Q_OBJECT
    Q_PROPERTY(QDate value READ value WRITE setValue)

public:
    QDate value() const { return m_value; }
    void setValue(const QDate &value) { m_value = value; }

private slots:
    void testValue();

private:
    QDate m_value;
};

class tst_DateTime : public QDjangoModel
{
    Q_OBJECT
    Q_PROPERTY(QDateTime value READ value WRITE setValue)

public:
    QDateTime value() const { return m_value; }
    void setValue(const QDateTime &value) { m_value = value; }

private slots:
    void testValue();

private:
    QDateTime m_value;
};

class tst_Double : public QDjangoModel
{
    Q_OBJECT
    Q_PROPERTY(double value READ value WRITE setValue)

public:
    double value() const { return m_value; }
    void setValue(double value) { m_value = value; }

private slots:
    void testValue();

private:
    double m_value;
};

class tst_Integer : public QDjangoModel
{
    Q_OBJECT
    Q_PROPERTY(int value READ value WRITE setValue)

public:
    int value() const { return m_value; }
    void setValue(int value) { m_value = value; }

private slots:
    void testValue();

private:
    int m_value;
};

class tst_LongLong : public QDjangoModel
{
    Q_OBJECT
    Q_PROPERTY(qlonglong value READ value WRITE setValue)

public:
    qlonglong value() const { return m_value; }
    void setValue(qlonglong value) { m_value = value; }

private slots:
    void testValue();

private:
    qlonglong m_value;
};

class tst_String : public QDjangoModel
{
    Q_OBJECT
    Q_PROPERTY(QString value READ value WRITE setValue)
    Q_CLASSINFO("value", "max_length=255")

public:
    QString value() const { return m_value; }
    void setValue(const QString &value) { m_value = value; }

private slots:
    void testValue();

private:
    QString m_value;
};

class tst_Time : public QDjangoModel
{
    Q_OBJECT
    Q_PROPERTY(QTime value READ value WRITE setValue)

public:
    QTime value() const { return m_value; }
    void setValue(const QTime &value) { m_value = value; }

private slots:
    void testValue();

private:
    QTime m_value;
};

