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

#include "QDjangoQuerySet.h"

#include "auth-models.h"

User::User(QObject *parent)
    : QDjangoModel(parent),
    m_isActive(true),
    m_isStaff(false),
    m_isSuperUser(false)
{
    // initialise dates
    m_dateJoined = QDateTime::currentDateTime();
    m_lastLogin = QDateTime::currentDateTime();
}

QString User::username() const
{
    return m_username;
}

void User::setUsername(const QString &username)
{
    m_username = username;
}

QString User::firstName() const
{
    return m_firstName;
}

void User::setFirstName(const QString &firstName)
{
    m_firstName = firstName;
}

QString User::lastName() const
{
    return m_lastName;
}

void User::setLastName(const QString &lastName)
{
    m_lastName = lastName;
}

QString User::email() const
{
    return m_email;
}

void User::setEmail(const QString &email)
{
    m_email = email;
}

QString User::password() const
{
    return m_password;
}

void User::setPassword(const QString &password)
{
    m_password = password;
}

bool User::isActive() const
{
    return m_isActive;
}

void User::setIsActive(bool isActive)
{
    m_isActive = isActive;
}

bool User::isStaff() const
{
    return m_isStaff;
}

void User::setIsStaff(bool isStaff)
{
    m_isStaff = isStaff;
}

bool User::isSuperUser() const
{
    return m_isSuperUser;
}

void User::setIsSuperUser(bool isSuperUser)
{
    m_isSuperUser = isSuperUser;
}

QDateTime User::dateJoined() const
{
    return m_dateJoined;
}

void User::setDateJoined(const QDateTime &dateJoined)
{
    m_dateJoined = dateJoined;
}

QDateTime User::lastLogin() const
{
    return m_lastLogin;
}

void User::setLastLogin(const QDateTime &lastLogin)
{
    m_lastLogin = lastLogin;
}

Group::Group(QObject *parent)
    : QDjangoModel(parent)
{
}

QString Group::name() const
{
    return m_name;
}

void Group::setName(const QString &name)
{
    m_name = name;
}

UserGroups::UserGroups(QObject *parent)
    : QDjangoModel(parent)
{
    setForeignKey("user", new User(this));
    setForeignKey("group", new Group(this));
}

User *UserGroups::user() const
{
    return qobject_cast<User*>(foreignKey("user"));
}

void UserGroups::setUser(User *user)
{
    setForeignKey("user", user);
}

Group *UserGroups::group() const
{
    return qobject_cast<Group*>(foreignKey("group"));
}

void UserGroups::setGroup(Group *group)
{
    setForeignKey("group", group);
}

Message::Message(QObject *parent)
    : QDjangoModel(parent)
{
    setForeignKey("user", new User(this));
}

/** Returns the User associated with this Message.
 */
User *Message::user() const
{
    return qobject_cast<User*>(foreignKey("user"));
}

void Message::setUser(User *user)
{
    setForeignKey("user", user);
}

QString Message::message() const
{
    return m_message;
}

void Message::setMessage(const QString &message)
{
    m_message = message;
}

Q_DECLARE_METATYPE(Group*)
Q_DECLARE_METATYPE(User*)
