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

#include <QObject>

class QDjangoHttpRequest;
class QDjangoHttpResponse;
class QDjangoUrlResolver;
class AdminControllerPrivate;
class ModelAdminFetcher;
class ModelAdminPrivate;

class AdminController : public QObject
{
    Q_OBJECT

public:
    AdminController(QObject* parent = 0);
    void setupUrls(QDjangoUrlResolver *urls);

public slots:
    QDjangoHttpResponse* index(const QDjangoHttpRequest &request);
    QDjangoHttpResponse* staticFiles(const QDjangoHttpRequest &request, const QString &path);

private:
    AdminControllerPrivate *d;
};

class ModelAdmin : public QObject
{
    Q_OBJECT

public:
    ModelAdmin(ModelAdminFetcher *fetcher, QObject *parent = 0);
    ~ModelAdmin();

    QList<QByteArray> changeFields() const;
    void setChangeFields(const QList<QByteArray> fields);

    QList<QByteArray> listFields() const;
    void setListFields(const QList<QByteArray> fields);

    QDjangoUrlResolver *urls() const;

public slots:
    QDjangoHttpResponse* addForm(const QDjangoHttpRequest &request);
    QDjangoHttpResponse* changeForm(const QDjangoHttpRequest &request, const QString &objectId);
    QDjangoHttpResponse* changeList(const QDjangoHttpRequest &request);
    QDjangoHttpResponse* deleteForm(const QDjangoHttpRequest &request, const QString &objectId);

private:
    ModelAdminPrivate *d;
};

