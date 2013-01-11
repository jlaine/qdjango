/*
 * Copyright (C) 2010-2013 Jeremy Lainé
 * Copyright (C) 2011 Mathias Hasselmann
 * Contact: http://code.google.com/p/qdjango/
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

#include <QSqlDriver>

#include "QDjango.h"
#include "QDjangoQuerySet.h"
#include "QDjangoWhere_p.h"

/// \cond

QDjangoCompiler::QDjangoCompiler(const char *modelName, const QSqlDatabase &db)
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

    const QString modelRef = QLatin1String("T") + QString::number(modelRefs.size());
    modelRefs.insert(modelPath, qMakePair(modelRef, *metaModel));
    return modelRef;
}

QString QDjangoCompiler::databaseColumn(const QString &name)
{
    QDjangoMetaModel model = baseModel;
    QString modelPath;
    QString modelRef = referenceModel(QString(), &model);

    QStringList bits = name.split(QLatin1String("__"));
    while (bits.size() > 1) {
        const QByteArray fk = bits.first().toLatin1();
        if (!model.foreignFields().contains(fk))
            break;

        QDjangoMetaModel foreignModel = QDjango::metaModel(model.foreignFields()[fk]);

        // store reference
        if (!modelPath.isEmpty())
            modelPath += QLatin1String("__");
        modelPath += bits.first();
        modelRef = referenceModel(modelPath, &foreignModel);

        model = foreignModel;
        bits.takeFirst();
    }

    const QDjangoMetaField field = model.localField(bits.join(QLatin1String("__")).toLatin1());
    return modelRef + QLatin1Char('.') + driver->escapeIdentifier(field.column(), QSqlDriver::FieldName);
}

QStringList QDjangoCompiler::fieldNames(bool recurse, QDjangoMetaModel *metaModel, const QString &modelPath)
{
    QStringList columns;
    if (!metaModel)
        metaModel = &baseModel;

    // store reference
    const QString tableName = referenceModel(modelPath, metaModel);
    foreach (const QDjangoMetaField &field, metaModel->localFields())
        columns << tableName + QLatin1Char('.') + driver->escapeIdentifier(field.column(), QSqlDriver::FieldName);
    if (!recurse)
        return columns;

    // recurse for foreign keys
    const QString pathPrefix = modelPath.isEmpty() ? QString() : (modelPath + QLatin1String("__"));
    foreach (const QByteArray &fkName, metaModel->foreignFields().keys()) {
        QDjangoMetaModel metaForeign = QDjango::metaModel(metaModel->foreignFields()[fkName]);
        columns += fieldNames(recurse, &metaForeign, pathPrefix + QString::fromLatin1(fkName));
    }
    return columns;
}

QString QDjangoCompiler::fromSql()
{
    QString from = driver->escapeIdentifier(baseModel.table(), QSqlDriver::TableName);
    foreach (const QString &name, modelRefs.keys()) {
        baseModel.localField(name.toLatin1() + QByteArray("_id"));
        from += QString::fromLatin1(" %1 %2 %3 ON %4.%5 = %6")
            .arg(baseModel.localField(name.toLatin1() + QByteArray("_id")).isNullable() ? "LEFT OUTER JOIN" : "INNER JOIN")
            .arg(driver->escapeIdentifier(modelRefs[name].second.table(), QSqlDriver::TableName))
            .arg(modelRefs[name].first)
            .arg(modelRefs[name].first)
            .arg(driver->escapeIdentifier(modelRefs[name].second.localField("pk").column(), QSqlDriver::FieldName))
            .arg(databaseColumn(name + QLatin1String("_id")));
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
        QString order = QLatin1String("ASC");
        if (field.startsWith(QLatin1Char('-'))) {
            order = QLatin1String("DESC");
            field = field.mid(1);
        } else if (field.startsWith(QLatin1Char('+'))) {
            field = field.mid(1);
        }
        bits.append(databaseColumn(field) + QLatin1Char(' ') + order);
    }
    if (!bits.isEmpty())
        limit += QLatin1String(" ORDER BY ") + bits.join(QLatin1String(", "));

    // limits
    if (highMark > 0)
        limit += QLatin1String(" LIMIT ") + QString::number(highMark - lowMark);
    if (lowMark > 0)
    {
        // no-limit is backend specific
        if (highMark <= 0)
            limit += QDjango::noLimitSql();
        limit += QLatin1String(" OFFSET ") + QString::number(lowMark);
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

QDjangoQuerySetPrivate::QDjangoQuerySetPrivate(const char *modelName)
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

    // execute query
    QDjangoQuery query(deleteQuery());
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

    const QStringList columns = compiler.fieldNames(selectRelated);
    const QString where = resolvedWhere.sql(db);
    const QString limit = compiler.orderLimitSql(orderBy, lowMark, highMark);
    QString sql = QLatin1String("SELECT ") + columns.join(QLatin1String(", ")) + QLatin1String(" FROM ") + compiler.fromSql();
    if (!where.isEmpty())
        sql += QLatin1String(" WHERE ") + where;
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
        for (int i = 0; i < columns.size(); ++i)
            props << query.value(i);
        properties.append(props);
    }
    hasResults = true;
    return true;
}

bool QDjangoQuerySetPrivate::sqlInsert(const QVariantMap &fields, QVariant *insertId)
{
    // execute query
    QDjangoQuery query(insertQuery(fields));
    if (!query.exec())
        return false;

    // fetch autoincrement pk
    if (insertId) {
        QSqlDatabase db = QDjango::database();
        if (db.driverName() == QLatin1String("QPSQL")) {
            const QDjangoMetaModel metaModel = QDjango::metaModel(m_modelName);
            QDjangoQuery query(db);
            const QDjangoMetaField primaryKey = metaModel.localField("pk");
            const QString seqName = db.driver()->escapeIdentifier(metaModel.table() + QLatin1Char('_') + primaryKey.column() + QLatin1String("_seq"), QSqlDriver::FieldName);
            if (!query.exec(QLatin1String("SELECT CURRVAL('") + seqName + QLatin1String("')")) || !query.next())
                return false;
            *insertId = query.value(0);
        } else {
            *insertId = query.lastInsertId();
        }
    }

    // invalidate cache
    if (hasResults) {
        properties.clear();
        hasResults = false;
    }

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

/** Returns the SQL query to perform a COUNT on the current set.
 */
QDjangoQuery QDjangoQuerySetPrivate::countQuery() const
{
    QSqlDatabase db = QDjango::database();

    // build query
    QDjangoCompiler compiler(m_modelName, db);
    QDjangoWhere resolvedWhere(whereClause);
    compiler.resolve(resolvedWhere);

    const QString where = resolvedWhere.sql(db);
    const QString limit = compiler.orderLimitSql(QStringList(), lowMark, highMark);
    QString sql = QLatin1String("SELECT COUNT(*) FROM ") + compiler.fromSql();
    if (!where.isEmpty())
        sql += QLatin1String(" WHERE ") + where;
    sql += limit;
    QDjangoQuery query(db);
    query.prepare(sql);
    resolvedWhere.bindValues(query);

    return query;
}

/** Returns the SQL query to perform a DELETE on the current set.
 */
QDjangoQuery QDjangoQuerySetPrivate::deleteQuery() const
{
    QSqlDatabase db = QDjango::database();

    // build query
    QDjangoCompiler compiler(m_modelName, db);
    QDjangoWhere resolvedWhere(whereClause);
    compiler.resolve(resolvedWhere);

    const QString where = resolvedWhere.sql(db);
    const QString limit = compiler.orderLimitSql(orderBy, lowMark, highMark);
    QString sql = QLatin1String("DELETE FROM ") + compiler.fromSql();
    if (!where.isEmpty())
        sql += QLatin1String(" WHERE ") + where;
    sql += limit;
    QDjangoQuery query(db);
    query.prepare(sql);
    resolvedWhere.bindValues(query);

    return query;
}

/** Returns the SQL query to perform an INSERT for the specified \a fields.
 */
QDjangoQuery QDjangoQuerySetPrivate::insertQuery(const QVariantMap &fields) const
{
    QSqlDatabase db = QDjango::database();
    const QDjangoMetaModel metaModel = QDjango::metaModel(m_modelName);

    // perform INSERT
    QStringList fieldColumns;
    QStringList fieldHolders;
    foreach (const QString &name, fields.keys()) {
        const QDjangoMetaField field = metaModel.localField(name.toLatin1());
        fieldColumns << db.driver()->escapeIdentifier(field.column(), QSqlDriver::FieldName);
        fieldHolders << QLatin1String("?");
    }

    QDjangoQuery query(db);
    query.prepare(QString::fromLatin1("INSERT INTO %1 (%2) VALUES(%3)").arg(
                  db.driver()->escapeIdentifier(metaModel.table(), QSqlDriver::TableName),
                  fieldColumns.join(QLatin1String(", ")), fieldHolders.join(QLatin1String(", "))));
    foreach (const QString &name, fields.keys())
        query.addBindValue(fields.value(name));
    return query;
}

/** Returns the SQL query to perform an UPDATE on the current set for the
    specified \a fields.
 */
QDjangoQuery QDjangoQuerySetPrivate::updateQuery(const QVariantMap &fields) const
{
    QSqlDatabase db = QDjango::database();
    const QDjangoMetaModel metaModel = QDjango::metaModel(m_modelName);

    // build query
    QDjangoCompiler compiler(m_modelName, db);
    QDjangoWhere resolvedWhere(whereClause);
    compiler.resolve(resolvedWhere);

    QString sql = QLatin1String("UPDATE ") + compiler.fromSql();

    // add SET
    QStringList fieldAssign;
    foreach (const QString &name, fields.keys()) {
        const QDjangoMetaField field = metaModel.localField(name.toLatin1());
        fieldAssign << db.driver()->escapeIdentifier(field.column(), QSqlDriver::FieldName) + QLatin1String(" = ?");
    }
    sql += QLatin1String(" SET ") + fieldAssign.join(QLatin1String(", "));

    // add WHERE
    const QString where = resolvedWhere.sql(db);
    if (!where.isEmpty())
        sql += QLatin1String(" WHERE ") + where;

    QDjangoQuery query(db);
    query.prepare(sql);
    foreach (const QString &name, fields.keys())
        query.addBindValue(fields.value(name));
    resolvedWhere.bindValues(query);

    return query;
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
        return -1;

    // execute query
    QDjangoQuery query(updateQuery(fields));
    if (!query.exec())
        return -1;

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
            fieldPos.insert(localFields[i].name(), i);
    } else {
        foreach (const QString &name, fields) {
            int pos = 0;
            foreach (const QDjangoMetaField &field, localFields) {
                if (field.name() == name)
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
                if (field.name() == name)
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

/// \endcond
