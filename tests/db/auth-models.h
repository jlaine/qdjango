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

#ifndef QDJANGO_AUTH_MODELS_H
#define QDJANGO_AUTH_MODELS_H

#include <QDateTime>

#include "QDjangoModel.h"

/** The User class represents a user in the authentication system.
 *
 *  It has a many-to-many relationship with the Group class.
 */
class User : public QDjangoModel
{
    Q_OBJECT
    Q_PROPERTY(QString username READ username WRITE setUsername)
    Q_PROPERTY(QString first_name READ firstName WRITE setFirstName)
    Q_PROPERTY(QString last_name READ lastName WRITE setLastName)
    Q_PROPERTY(QString email READ email WRITE setEmail)
    Q_PROPERTY(QString password READ password WRITE setPassword)
    Q_PROPERTY(bool is_active READ isActive WRITE setIsActive)
    Q_PROPERTY(bool is_staff READ isStaff WRITE setIsStaff)
    Q_PROPERTY(bool is_superuser READ isSuperUser WRITE setIsSuperUser)
    Q_PROPERTY(QDateTime date_joined READ dateJoined WRITE setDateJoined)
    Q_PROPERTY(QDateTime last_login READ lastLogin WRITE setLastLogin)

    Q_CLASSINFO("username", "max_length=30")
    Q_CLASSINFO("first_name", "max_length=30")
    Q_CLASSINFO("last_name", "max_length=30")
    Q_CLASSINFO("password", "max_length=128")

public:
    User(QObject *parent = 0);

    QString username() const;
    void setUsername(const QString &username);

    QString firstName() const;
    void setFirstName(const QString &firstName);

    QString lastName() const;
    void setLastName(const QString &lastName);

    QString email() const;
    void setEmail(const QString &email);

    QString password() const;
    void setPassword(const QString &password);

    // flags
    bool isActive() const;
    void setIsActive(bool isActive);

    bool isStaff() const;
    void setIsStaff(bool isStaff);

    bool isSuperUser() const;
    void setIsSuperUser(bool isSuperUser);

    // dates
    QDateTime dateJoined() const;
    void setDateJoined(const QDateTime &dateJoined);

    QDateTime lastLogin() const;
    void setLastLogin(const QDateTime &lastLogin);

private:
    QString m_username;
    QString m_firstName;
    QString m_lastName;
    QString m_email;
    QString m_password;
    bool m_isActive;
    bool m_isStaff;
    bool m_isSuperUser;
    QDateTime m_dateJoined;
    QDateTime m_lastLogin;
};

/** The Group class represents a group in the authentication system.
 *
 *  It has a many-to-many relationship with the User class.
 */
class Group : public QDjangoModel
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName)

public:
    Group(QObject *parent = 0);
    QString name() const;
    void setName(const QString &name);

private:
    QString m_name;
};

class UserGroups : public QDjangoModel
{
    Q_OBJECT
    Q_PROPERTY(User* user READ user WRITE setUser);
    Q_PROPERTY(Group* group READ group WRITE setGroup);

public:
    UserGroups(QObject *parent = 0);

    User *user() const;
    void setUser(User *user);

    Group *group() const;
    void setGroup(Group *group);
};

/** The Message class represents a message for a given User.
 */
class Message : public QDjangoModel
{
    Q_OBJECT
    Q_PROPERTY(User* user READ user WRITE setUser);
    Q_PROPERTY(QString message READ message WRITE setMessage)

public:
    Message(QObject *parent = 0);

    User *user() const;
    void setUser(User *user);

    QString message() const;
    void setMessage(const QString &message);

private:
    QString m_message;
};

#endif
