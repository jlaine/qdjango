/*
 * Copyright (C) 2010-2015 Jeremy Lainé
 * Contact: https://github.com/jlaine/qdjango
 *
 * This file is part of the QDjango Library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 */

#include <QDate>

#include "QDjangoModel.h"

#include "auth-models.h"

class tst_QDjangoMetaModel : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void testBool();
    void testByteArray();
    void testDate();
    void testDateTime();
    void testDouble();
    void testInteger();
    void testLongLong();
    void testString();
    void testTime();
    void testOptions();
    void testConstraints();
    void testIsValid();
};

class tst_Bool : public QDjangoModel
{
    Q_OBJECT
    Q_PROPERTY(bool value READ value WRITE setValue)

public:
    bool value() const { return m_value; }
    void setValue(bool value) { m_value = value; }

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

private:
    QTime m_value;
};

class tst_Options : public QDjangoModel
{
    Q_OBJECT
    Q_PROPERTY(int aField READ aField WRITE setAField)
    Q_PROPERTY(int bField READ bField WRITE setBField)
    Q_PROPERTY(int blankField READ blankField WRITE setBlankField)
    Q_PROPERTY(int ignoredField READ ignoredField WRITE setIgnoredField)
    Q_PROPERTY(int indexField READ indexField WRITE setIndexField)
    Q_PROPERTY(int nullField READ nullField WRITE setNullField)
    Q_PROPERTY(int uniqueField READ uniqueField WRITE setUniqueField)

    Q_CLASSINFO("__meta__", "db_table=some_table unique_together=aField,bField")
    Q_CLASSINFO("bField", "db_column=b_field")
    Q_CLASSINFO("blankField", "blank=true")
    Q_CLASSINFO("ignoredField", "ignore_field=true")
    Q_CLASSINFO("indexField", "db_index=true")
    Q_CLASSINFO("nullField", "null=true")
    Q_CLASSINFO("uniqueField", "unique=true")

public:
    int aField() const { return m_aField; }
    void setAField(int value) { m_aField = value; }

    int bField() const { return m_bField; }
    void setBField(int value) { m_bField = value; }

    int blankField() const { return m_blankField; }
    void setBlankField(int value) { m_blankField = value; }

    int ignoredField() const { return m_ignoredField; }
    void setIgnoredField(int value) { m_ignoredField = value; }

    int indexField() const { return m_indexField; }
    void setIndexField(int value) { m_indexField = value; }

    int nullField() const { return m_nullField; }
    void setNullField(int value) { m_nullField = value; }

    int uniqueField() const { return m_uniqueField; }
    void setUniqueField(int value) { m_uniqueField = value; }

private:
    int m_aField;
    int m_bField;
    int m_blankField;
    int m_ignoredField;
    int m_indexField;
    int m_nullField;
    int m_uniqueField;
};

class tst_FkConstraint : public QDjangoModel
{
    Q_OBJECT
    Q_PROPERTY(User *noConstraint READ noConstraint WRITE setNoConstraint)
    Q_PROPERTY(User *cascadeConstraint READ cascadeConstraint WRITE setCascadeConstraint)
    Q_PROPERTY(Group *nullConstraint READ nullConstraint WRITE setNullConstraint)

    Q_CLASSINFO("cascadeConstraint", "on_delete=cascade")
    Q_CLASSINFO("restrictConstraint", "on_delete=restrict")
    Q_CLASSINFO("nullConstraint", "null=true on_delete=set_null")

public:
    tst_FkConstraint(QObject *parent = 0);

    User *noConstraint() const;
    void setNoConstraint(User *user);

    User *cascadeConstraint() const;
    void setCascadeConstraint(User *user);

    Group *nullConstraint() const;
    void setNullConstraint(Group *group);
};

class tst_FkConstraintWithRestrict : public tst_FkConstraint
{
    Q_OBJECT
    Q_PROPERTY(User *restrictConstraint READ restrictConstraint WRITE setRestrictConstraint)

    Q_CLASSINFO("__meta__", "db_table=tst_fkconstraint")
    Q_CLASSINFO("restrictConstraint", "on_delete=restrict")
public:
    tst_FkConstraintWithRestrict(QObject *parent = 0);

    User *restrictConstraint() const;
    void setRestrictConstraint(User *user);
};
