/*
 * Copyright (C) 2010-2012 Jeremy Lainé
 * Copyright (C) 2011 Mathias Hasselmann
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

#include <QSqlDriver>

#include "QDjango.h"
#include "QDjangoQuerySet.h"
#include "QDjangoWhere_p.h"

QDjangoCompiler::QDjangoCompiler(const QString &modelName, const QSqlDatabase &db)
{
    driver = db.driver();
    baseModel = QDjango::metaModel(modelName);
}

QString QDjangoCompiler::referenceModel(const QString &modelPath, QDjangoMetaModel *metaModel)
{
    if (modelPath.isEmpty())
        return driver->escapeIdentifier(baseModel.table(), QSqlDriver::TableName);

    if (modelRefs.contains(modelPath))
        return modelRefs.value(modelPath).first;

    const QString modelRef = "T" + QString::number(modelRefs.size());
    modelRefs.insert(modelPath, qMakePair(modelRef, *metaModel));
    return modelRef;
}

QString QDjangoCompiler::databaseColumn(const QString &name)
{
    QDjangoMetaModel model = baseModel;
    QString modelPath;
    QString modelRef = referenceModel(QString(), &model);

    QStringList bits = name.split("__");
    while (bits.size() > 1) {
        const QByteArray fk = bits.first().toLatin1();
        if (!model.foreignFields().contains(fk))
            break;

        QDjangoMetaModel foreignModel = QDjango::metaModel(model.foreignFields()[fk]);

        // store reference
        if (!modelPath.isEmpty())
            modelPath += "__";
        modelPath += bits.first();
        modelRef = referenceModel(modelPath, &foreignModel);

        model = foreignModel;
        bits.takeFirst();
    }

    QString fieldName = bits.join("__");
    if (fieldName == QLatin1String("pk"))
        fieldName = model.primaryKey();

    return modelRef + "." + driver->escapeIdentifier(fieldName, QSqlDriver::FieldName);
}

QStringList QDjangoCompiler::fieldNames(bool recurse, QDjangoMetaModel *metaModel, const QString &modelPath)
{
    QStringList fields;
    if (!metaModel)
        metaModel = &baseModel;

    // store reference
    const QString tableName = referenceModel(modelPath, metaModel);
    foreach (const QDjangoMetaField &field, metaModel->localFields())
        fields << tableName + "." + driver->escapeIdentifier(field.name, QSqlDriver::FieldName);
    if (!recurse)
        return fields;

    // recurse for foreign keys
    const QString pathPrefix = modelPath.isEmpty() ? QString() : (modelPath + "__");
    foreach (const QByteArray &fkName, metaModel->foreignFields().keys()) {
        QDjangoMetaModel metaForeign = QDjango::metaModel(metaModel->foreignFields()[fkName]);
        fields += fieldNames(recurse, &metaForeign, pathPrefix + fkName);
    }
    return fields;
}

QString QDjangoCompiler::fromSql()
{
    QString from = driver->escapeIdentifier(baseModel.table(), QSqlDriver::TableName);
    foreach (const QString &name, modelRefs.keys()) {
        from += QString(" INNER JOIN %1 %2 ON %3.%4 = %5")
            .arg(driver->escapeIdentifier(modelRefs[name].second.table(), QSqlDriver::TableName))
            .arg(modelRefs[name].first)
            .arg(modelRefs[name].first)
            .arg(driver->escapeIdentifier(modelRefs[name].second.primaryKey(), QSqlDriver::FieldName))
            .arg(databaseColumn(name + "_id"));
    }
    return from;
}

QString QDjangoCompiler::orderLimitSql(const QStringList orderBy, int lowMark, int highMark)
{
    QString limit;

    // order
    QStringList bits;
    QString field;
    foreach (field, orderBy) {
        QString order = "ASC";
        if (field.startsWith("-")) {
            order = "DESC";
            field = field.mid(1);
        } else if (field.startsWith("+")) {
            field = field.mid(1);
        }
        bits.append(QString("%1 %2").arg(databaseColumn(field), order));
    }
    if (!bits.isEmpty())
        limit += " ORDER BY " + bits.join(", ");

    // limits
    if (highMark > 0)
        limit += QString(" LIMIT %1").arg(highMark - lowMark);
    if (lowMark > 0)
    {
        // no-limit is backend specific
        if (highMark <= 0)
            limit += QDjango::noLimitSql();
        limit += QString(" OFFSET %1").arg(lowMark);
    }
    return limit;
}

void QDjangoCompiler::resolve(QDjangoWhere &where)
{
    // resolve column
    if (where.d->operation != QDjangoWhere::None)
        where.d->key = databaseColumn(where.d->key);

    // recurse into children
    for (int i = 0; i < where.d->children.size(); i++)
        resolve(where.d->children[i]);
}

QDjangoQuerySetPrivate::QDjangoQuerySetPrivate(const QString &modelName)
    : counter(1),
    hasResults(false),
    lowMark(0),
    highMark(0),
    selectRelated(false),
    m_modelName(modelName)
{
}

void QDjangoQuerySetPrivate::addFilter(const QDjangoWhere &where)
{
    // it is not possible to add filters once a limit has been set
    Q_ASSERT(!lowMark && !highMark);

    whereClause = whereClause && where;
}

QDjangoWhere QDjangoQuerySetPrivate::resolvedWhere(const QSqlDatabase &db) const
{
    QDjangoCompiler compiler(m_modelName, db);
    QDjangoWhere resolvedWhere(whereClause);
    compiler.resolve(resolvedWhere);
    return resolvedWhere;
}

int QDjangoQuerySetPrivate::sqlCount() const
{
    QSqlDatabase db = QDjango::database();

    // build query
    QDjangoCompiler compiler(m_modelName, db);
    QDjangoWhere resolvedWhere(whereClause);
    compiler.resolve(resolvedWhere);

    const QString where = resolvedWhere.sql(db);
    const QString limit = compiler.orderLimitSql(QStringList(), lowMark, highMark);
    QString sql = "SELECT COUNT(*) FROM " + compiler.fromSql();
    if (!where.isEmpty())
        sql += " WHERE " + where;
    sql += limit;
    QDjangoQuery query(db);
    query.prepare(sql);
    resolvedWhere.bindValues(query);

    // execute query
    if (!query.exec() || !query.next())
        return -1;
    return query.value(0).toInt();
}

bool QDjangoQuerySetPrivate::sqlDelete()
{
    // DELETE on an empty queryset doesn't need a query
    if (whereClause.isNone())
        return true;

    // FIXME : it is not possible to remove entries once a limit has been set
    // because SQLite does not support limits on DELETE unless compiled with the
    // SQLITE_ENABLE_UPDATE_DELETE_LIMIT option
    if (lowMark || highMark)
        return false;

    QSqlDatabase db = QDjango::database();

    // build query
    QDjangoCompiler compiler(m_modelName, db);
    QDjangoWhere resolvedWhere(whereClause);
    compiler.resolve(resolvedWhere);

    const QString where = resolvedWhere.sql(db);
    const QString limit = compiler.orderLimitSql(orderBy, lowMark, highMark);
    QString sql = "DELETE FROM " + compiler.fromSql();
    if (!where.isEmpty())
        sql += " WHERE " + where;
    sql += limit;
    QDjangoQuery query(db);
    query.prepare(sql);
    resolvedWhere.bindValues(query);

    // execute query
    if (!query.exec())
        return false;

    // invalidate cache
    if (hasResults) {
        properties.clear();
        hasResults = false;
    }
    return true;
}

bool QDjangoQuerySetPrivate::sqlFetch()
{
    if (hasResults || whereClause.isNone())
        return true;

    QSqlDatabase db = QDjango::database();

    // build query
    QDjangoCompiler compiler(m_modelName, db);
    QDjangoWhere resolvedWhere(whereClause);
    compiler.resolve(resolvedWhere);

    const QStringList fields = compiler.fieldNames(selectRelated);
    const QString where = resolvedWhere.sql(db);
    const QString limit = compiler.orderLimitSql(orderBy, lowMark, highMark);
    QString sql = "SELECT " + fields.join(", ") + " FROM " + compiler.fromSql();
    if (!where.isEmpty())
        sql += " WHERE " + where;
    sql += limit;
    QDjangoQuery query(db);
    query.prepare(sql);
    resolvedWhere.bindValues(query);

    // execute query
    if (!query.exec())
        return false;

    // store results
    while (query.next()) {
        QVariantList props;
        for (int i = 0; i < fields.size(); ++i)
            props << query.value(i);
        properties.append(props);
    }
    hasResults = true;
    return true;
}

bool QDjangoQuerySetPrivate::sqlLoad(QObject *model, int index)
{
    if (!sqlFetch())
        return false;

    if (index < 0 || index >= properties.size())
    {
        qWarning("QDjangoQuerySet out of bounds");
        return false;
    }

    const QDjangoMetaModel metaModel = QDjango::metaModel(m_modelName);
    int pos = 0;
    metaModel.load(model, properties.at(index), pos);
    return true;
}

int QDjangoQuerySetPrivate::sqlUpdate(const QVariantMap &fields)
{
    // UPDATE on an empty queryset doesn't need a query
    if (whereClause.isNone() || fields.isEmpty())
        return 0;

    // FIXME : it is not possible to update entries once a limit has been set
    // because SQLite does not support limits on UPDATE unless compiled with the
    // SQLITE_ENABLE_UPDATE_DELETE_LIMIT option
    if (lowMark || highMark)
        return 0;

    QSqlDatabase db = QDjango::database();

    // build query
    QDjangoCompiler compiler(m_modelName, db);
    QDjangoWhere resolvedWhere(whereClause);
    compiler.resolve(resolvedWhere);

    QString sql = "UPDATE " + compiler.fromSql();

    // add SET
    QStringList fieldAssign;
    foreach (const QString &name, fields.keys())
        fieldAssign << db.driver()->escapeIdentifier(name, QSqlDriver::FieldName) + " = ?";
    sql += " SET " + fieldAssign.join(", ");

    // add WHERE
    const QString where = resolvedWhere.sql(db);
    if (!where.isEmpty())
        sql += " WHERE " + where;

    QDjangoQuery query(db);
    query.prepare(sql);
    foreach (const QString &name, fields.keys())
        query.addBindValue(fields.value(name));
    resolvedWhere.bindValues(query);

    // execute query
    if (!query.exec())
        return 0;

    // invalidate cache
    if (hasResults) {
        properties.clear();
        hasResults = false;
    }

    return query.numRowsAffected();
}

QList<QVariantMap> QDjangoQuerySetPrivate::sqlValues(const QStringList &fields)
{
    QList<QVariantMap> values;
    if (!sqlFetch())
        return values;

    const QDjangoMetaModel metaModel = QDjango::metaModel(m_modelName);

    // build field list
    const QList<QDjangoMetaField> localFields = metaModel.localFields();
    QMap<QString, int> fieldPos;
    if (fields.isEmpty()) {
        for (int i = 0; i < localFields.size(); ++i)
            fieldPos.insert(QString::fromAscii(localFields[i].name), i);
    } else {
        foreach (const QString &name, fields) {
            int pos = 0;
            foreach (const QDjangoMetaField &field, localFields) {
                if (field.name == name)
                    break;
                pos++;
            }
            Q_ASSERT_X(pos < localFields.size(), "QDjangoQuerySet<T>::values", "unknown field requested");
            fieldPos.insert(name, pos);
        }
    }

    // extract values
    foreach (const QVariantList &props, properties) {
        QVariantMap map;
        QMap<QString, int>::const_iterator i;
        for (i = fieldPos.constBegin(); i != fieldPos.constEnd(); ++i)
            map[i.key()] = props[i.value()];
        values.append(map);
    }
    return values;
}

QList<QVariantList> QDjangoQuerySetPrivate::sqlValuesList(const QStringList &fields)
{
    QList<QVariantList> values;
    if (!sqlFetch())
        return values;

    const QDjangoMetaModel metaModel = QDjango::metaModel(m_modelName);

    // build field list
    const QList<QDjangoMetaField> localFields = metaModel.localFields();
    QList<int> fieldPos;
    if (fields.isEmpty()) {
        for (int i = 0; i < localFields.size(); ++i)
            fieldPos << i;
    } else {
        foreach (const QString &name, fields) {
            int pos = 0;
            foreach (const QDjangoMetaField &field, localFields) {
                if (field.name == name)
                    break;
                pos++;
            }
            Q_ASSERT_X(pos < localFields.size(), "QDjangoQuerySet<T>::valuesList", "unknown field requested");
            fieldPos << pos;
        }
    }

    // extract values
    foreach (const QVariantList &props, properties) {
        QVariantList list;
        foreach (int pos, fieldPos)
            list << props.at(pos);
        values.append(list);
    }
    return values;
}

