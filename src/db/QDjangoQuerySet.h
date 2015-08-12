/*
 * Copyright (C) 2010-2015 Jeremy Lainé
 * Copyright (C) 2011 Mathias Hasselmann
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

#ifndef QDJANGO_QUERYSET_H
#define QDJANGO_QUERYSET_H

#include "QDjango.h"
#include "QDjangoWhere.h"
#include "QDjangoQuerySet_p.h"

/** \brief The QDjangoQuerySet class is a template class for performing
 *   database queries.
 *
 *  The QDjangoQuerySet template class allows you to define and manipulate
 *  sets of QDjangoModel objects stored in the database.
 *
 *  You can chain filter expressions using the filter() and exclude() methods
 *  or apply limits on the number of rows using the limit() method.
 *
 *  You can retrieve database values using the values() and valuesList()
 *  methods or retrieve model instances using the get() and at() methods.
 *
 *  You can also delete sets of objects using the remove() method.
 *
 *  Behinds the scenes, the QDjangoQuerySet class uses implicit sharing to
 *  reduce memory usage and avoid needless copying of data.
 *
 * \ingroup Database
 */
template <class T>
    class QDjangoQuerySet
{
public:
    /** \cond declarations for STL-style container algorithms */
    typedef int size_type;
    typedef T value_type;
    typedef value_type *pointer;
    typedef const value_type *const_pointer;
    typedef value_type &reference;
    typedef const value_type &const_reference;
    typedef qptrdiff difference_type;
    /** \endcond */

    /** The QDjangoQuerySet::const_iterator class provides an STL-style const iterator
     *  for QDjangoQuerySet.
     *
     *  QDjangoQuerySet::const_iterator allows you to iterate over a QDjangoQuerySet.
     *  As a const iterator it doesn't permit you to modify the QDjangoQuerySet.
     *
     *  The default QDjangoQuerySet::const_iterator constructor creates an uninitialized iterator. You must
     *  initialize it using a QDjangoQuerySet function like QDjangoQuerySet::constBegin(), or
     *  QDjangoQuerySet::constEnd() before you can start iterating. Here's a typical loop that
     *  prints all the objects stored in a set:
     *
     *  \code
     *  QDjangoQuerySet<Weblog::Post> posts;
     *
     *  foreach(const Weblog::Post &p, posts) {
     *      cout << p << endl;
     *  }
     *  \endcode
     */
    class const_iterator
    {
        friend class QDjangoQuerySet;

    public:
        /** A synonym for std::bidirectional_iterator_tag indicating this iterator
         *  permits bidirectional access.
         */
        typedef std::bidirectional_iterator_tag  iterator_category;

        /** \cond declarations for STL-style container algorithms */
        typedef qptrdiff difference_type;
        typedef T value_type;
        typedef T *pointer;
        typedef T &reference;
        /** \endcond */

        /** Constructs an uninitialized iterator.
         *
         *  Functions like operator*() and operator++() should not be called on an uninitialized
         *  iterator. Use const_iterator::operator=() to assign a value to it before using it.
         *
         *  \sa See also QDjangoQuerySet::constBegin() and QDjangoQuerySet::constEnd().
         */
        const_iterator()
            : m_querySet(0)
            , m_fetched(-1)
            , m_offset(0)
        {
        }

        /** Constructs a copy of \p other.
         */
        const_iterator(const const_iterator &other)
            : m_querySet(other.m_querySet)
            , m_fetched(-1)
            , m_offset(other.m_offset)
        {
        }

    private:
        const_iterator(const QDjangoQuerySet<T> *querySet, int offset = 0)
            : m_querySet(querySet)
            , m_fetched(-1)
            , m_offset(offset)
        {
        }

    public:
        /** Returns the current item.
         *
         *  \sa operator->()
         */
        const T &operator*() const { return *t(); }

        /** Returns a pointer to the current item.
         *
         *  \sa operator*()
         */
        const T *operator->() const { return t(); }


        /** Returns \c true if \p other points to the same item as this iterator;
         *  otherwise returns \c false.
         *
         *  \sa operator!=()
         */
        bool operator==(const const_iterator &other) const
        {
            return m_querySet == other.m_querySet && m_offset == other.m_offset;
        }

        /** Returns \c true if \p other points to a different item than this iterator;
         *  otherwise returns \c false.
         *
         *  \sa operator==()
         */
        bool operator!=(const const_iterator &other) const
        {
            return m_querySet != other.m_querySet || m_offset != other.m_offset;
        }

        /** Returns \c true if other \p points to a position behind this iterator;
         *  otherwise returns \c false.
         */
        bool operator<(const const_iterator& other) const
        {
            return (m_querySet == other.m_querySet && m_offset < other.m_offset)
                    || m_querySet < other.m_querySet;
        }

        /** Returns \c true if other \p points to a position behind or equal this iterator;
         *  otherwise returns \c false.
         */
        bool operator<=(const const_iterator& other) const
        {
            return (m_querySet == other.m_querySet && m_offset <= other.m_offset)
                    || m_querySet < other.m_querySet;
        }

        /** Returns \c true if other \p points to a position before this iterator;
         *  otherwise returns \c false.
         */
        bool operator>(const const_iterator& other) const
        {
            return (m_querySet == other.m_querySet && m_offset > other.m_offset)
                    || m_querySet > other.m_querySet;
        }

        /** Returns \c true if other \p points to a position before or equal this iterator;
         *  otherwise returns \c false.
         */
        bool operator>=(const const_iterator& other) const
        {
            return (m_querySet == other.m_querySet && m_offset >= other.m_offset)
                    || m_querySet > other.m_querySet;
        }

        /** The prefix ++ operator (\c ++it) advances the iterator to the next item in the set
         *  and returns an iterator to the new current item.
         *
         *  Calling this function on QDjangoQuerySet::end() leads to undefined results.
         *
         *  \sa operator--()
         */
        const_iterator &operator++() { ++m_offset; return *this; }

        /** The postfix ++ operator (\c it++) advances the iterator to the next item in the set and
         *  returns an iterator to the previously current item.
         *
         *  Calling this function on QDjangoQuerySet::end() leads to undefined results.
         *
         *  \sa operator--(int)
         */
        const_iterator operator++(int) { const_iterator n(*this); ++m_offset; return n; }

        /** Advances the iterator by \p i items.
         * (If \p i is negative, the iterator goes backward.)
         *
         *  \sa operator-=() and operator+().
         */
        const_iterator &operator+=(int i) { m_offset += i; return *this; }

        /** Returns an iterator to the item at \p i positions forward from this iterator.
         * (If \p i is negative, the iterator goes backward.)
         *
         *  \sa operator-() and operator+=()
         */
        const_iterator operator+(int i) const { return const_iterator(m_querySet, m_offset + i); }

        /** Makes the iterator go back by \p i items.
         * (If \p i is negative, the iterator goes forward.)
         *
         * \sa operator+=() and operator-()
         */
        const_iterator &operator-=(int i) { m_offset -= i; return *this; }

        /** Returns an iterator to the item at \p i positions backward from this iterator.
         * (If \p i is negative, the iterator goes forward.)
         *
         *  \sa operator+() and operator-=()
         */
        const_iterator operator-(int i) const { return const_iterator(m_querySet, m_offset - i); }

        /** The prefix -- operator (\c --it) makes the preceding item current
         *  and returns an iterator to the new current item.
         *
         *  Calling this function on QDjangoQuerySet::begin() leads to undefined results.
         *
         *  \sa operator++().
         */
        const_iterator &operator--() { --m_offset; return *this; }

        /** The postfix -- operator (\c it--) makes the preceding item current
         *  and returns an iterator to the previously current item.
         *
         *  Calling this function on QDjangoQuerySet::begin() leads to undefined results.
         *
         *  \sa operator++(int).
         */
        const_iterator operator--(int) { const_iterator n(*this); --m_offset; return n; }


        /** Returns the number of items between the item pointed to by \p other
         *  and the item pointed to by this iterator.
         */
        difference_type operator-(const const_iterator &other) const { return m_offset - other.m_offset; }

    private:
        const T *t() const
        {
            if (m_fetched != m_offset && m_querySet) {
                if (const_cast<QDjangoQuerySet<T> *>(m_querySet)->at(m_offset, &m_object)) {
                    m_fetched = m_offset;
                }
            }

            return m_fetched == m_offset ? &m_object : 0;
        }

    private:
        const QDjangoQuerySet<T> *m_querySet;
        mutable int m_fetched;
        mutable T m_object;

        int m_offset;
    };

    /** Qt-style synonym for QDjangoQuerySet::const_iterator. */
    typedef const_iterator ConstIterator;

    QDjangoQuerySet();
    QDjangoQuerySet(const QDjangoQuerySet<T> &other);
    ~QDjangoQuerySet();

    QDjangoQuerySet all() const;
    QDjangoQuerySet exclude(const QDjangoWhere &where) const;
    QDjangoQuerySet filter(const QDjangoWhere &where) const;
    QDjangoQuerySet limit(int pos, int length = -1) const;
    QDjangoQuerySet none() const;
    QDjangoQuerySet orderBy(const QStringList &keys) const;
    QDjangoQuerySet selectRelated() const;

    int count() const;
    QDjangoWhere where() const;

    bool remove();
    int size();
    int update(const QVariantMap &fields);
    QList<QVariantMap> values(const QStringList &fields = QStringList());
    QList<QVariantList> valuesList(const QStringList &fields = QStringList());

    T *get(const QDjangoWhere &where, T *target = 0) const;
    T *at(int index, T *target = 0);

    const_iterator constBegin() const;
    const_iterator begin() const;

    const_iterator constEnd() const;
    const_iterator end() const;

    QDjangoQuerySet<T> &operator=(const QDjangoQuerySet<T> &other);

private:
    QDjangoQuerySetPrivate *d;
};

/** Constructs a new queryset.
 */
template <class T>
QDjangoQuerySet<T>::QDjangoQuerySet()
{
    d = new QDjangoQuerySetPrivate(T::staticMetaObject.className());
}

/** Constructs a copy of \a other.
 *
 * \param other
 */
template <class T>
QDjangoQuerySet<T>::QDjangoQuerySet(const QDjangoQuerySet<T> &other)
{
    other.d->counter.ref();
    d = other.d;
}

/** Destroys the queryset.
 */
template <class T>
QDjangoQuerySet<T>::~QDjangoQuerySet()
{
    if (!d->counter.deref())
        delete d;
}

/** Returns the object in the QDjangoQuerySet at the given index.
 *
 *  Returns 0 if the index is out of bounds.
 *
 *  If target is 0, a new object instance will be allocated which
 *  you must free yourself.
 *
 * \param index
 * \param target optional existing model instance.
 */
template <class T>
T *QDjangoQuerySet<T>::at(int index, T *target)
{
    T *entry = target ? target : new T;
    if (!d->sqlLoad(entry, index))
    {
        if (!target)
            delete entry;
        return 0;
    }
    return entry;
}

/** Returns a const STL-style iterator pointing to the first object in the QDjangoQuerySet.
 *
 *  \sa begin() and constEnd().
 */
template <class T>
typename QDjangoQuerySet<T>::const_iterator QDjangoQuerySet<T>::constBegin() const
{
    return const_iterator(this);
}

/** Returns a const STL-style iterator pointing to the first object in the QDjangoQuerySet.
 *
 *  \sa constBegin() and end().
 */
template <class T>
typename QDjangoQuerySet<T>::const_iterator QDjangoQuerySet<T>::begin() const
{
    return const_iterator(this);
}

/** Returns a const STL-style iterator pointing to the imaginary object after the last
 *  object in the QDjangoQuerySet.
 *
 *  \sa constBegin() and end().
 */
template <class T>
typename QDjangoQuerySet<T>::const_iterator QDjangoQuerySet<T>::constEnd() const
{
    return const_iterator(this, QDjangoQuerySet<T>::count());
}

/** Returns a const STL-style iterator pointing to the imaginary object after the last
 *  object in the QDjangoQuerySet.
 *
 *  \sa begin() and constEnd().
 */
template <class T>
typename QDjangoQuerySet<T>::const_iterator QDjangoQuerySet<T>::end() const
{
    return const_iterator(this, QDjangoQuerySet<T>::count());
}

/** Returns a copy of the current QDjangoQuerySet.
 */
template <class T>
QDjangoQuerySet<T> QDjangoQuerySet<T>::all() const
{
    QDjangoQuerySet<T> other;
    other.d->lowMark = d->lowMark;
    other.d->highMark = d->highMark;
    other.d->orderBy = d->orderBy;
    other.d->selectRelated = d->selectRelated;
    other.d->whereClause = d->whereClause;
    return other;
}

/** Counts the number of objects in the queryset using an SQL COUNT query,
 *  or -1 if the query failed.
 *
 *  If you intend to iterate over the results, you should consider using
 *  size() instead.
 *
 * \note If the QDjangoQuerySet is already fully fetched, this simply returns
 *  the number of objects.
 */
template <class T>
int QDjangoQuerySet<T>::count() const
{
    if (d->hasResults)
        return d->properties.size();

    // execute COUNT query
    QDjangoQuery query(d->countQuery());
    if (!query.exec() || !query.next())
        return -1;
    return query.value(0).toInt();
}

/** Returns a new QDjangoQuerySet containing objects for which the given key
 *  where condition is false.
 *
 *  You can chain calls to filter() and exclude() to further refine the
 *  filtering conditions.
 *
 * \param where QDjangoWhere expressing the exclude condition
 *
 * \sa filter()
 */
template <class T>
QDjangoQuerySet<T> QDjangoQuerySet<T>::exclude(const QDjangoWhere &where) const
{
    QDjangoQuerySet<T> other = all();
    other.d->addFilter(!where);
    return other;
}

/** Returns a new QDjangoQuerySet containing objects for which the given
 *  where condition is true.
 *
 *  You can chain calls to filter() and exclude() to progressively refine
 *  your filtering conditions.
 *
 * \param where QDjangoWhere expressing the filter condition
 *
 * \sa exclude()
 */
template <class T>
QDjangoQuerySet<T> QDjangoQuerySet<T>::filter(const QDjangoWhere &where) const
{
    QDjangoQuerySet<T> other = all();
    other.d->addFilter(where);
    return other;
}

/** Returns the object in the QDjangoQuerySet for which the given
 *  where condition is true.
 *
 *  Returns 0 if the number of matching object is not exactly one.
 *
 *  If target is 0, a new object instance will be allocated which
 *  you must free yourself.
 *
 * \param where QDjangoWhere expressing the lookup condition
 * \param target optional existing model instance.
 */
template <class T>
T *QDjangoQuerySet<T>::get(const QDjangoWhere &where, T *target) const
{
    QDjangoQuerySet<T> qs = filter(where);
    return qs.size() == 1 ? qs.at(0, target) : 0;
}

/** Returns a new QDjangoQuerySet containing limiting the number of
 *  records to manipulate.
 *
 *  You can chain calls to limit() to further restrict the number
 *  of returned records.
 *
 *  However, you cannot apply additional restrictions using filter(),
 *  exclude(), get(), orderBy() or remove() on the returned QDjangoQuerySet.
 *
 * \param pos offset of the records
 * \param length maximum number of records
 */
template <class T>
QDjangoQuerySet<T> QDjangoQuerySet<T>::limit(int pos, int length) const
{
    Q_ASSERT(pos >= 0);
    Q_ASSERT(length >= -1);

    QDjangoQuerySet<T> other = all();
    other.d->lowMark += pos;
    if (length > 0)
    {
        // calculate new high mark
        other.d->highMark = other.d->lowMark + length;
        // never exceed the current high mark
        if (d->highMark > 0 && other.d->highMark > d->highMark)
            other.d->highMark = d->highMark;
    }
    return other;
}

/** Returns an empty QDjangoQuerySet.
 */
template <class T>
QDjangoQuerySet<T> QDjangoQuerySet<T>::none() const
{
    QDjangoQuerySet<T> other;
    other.d->whereClause = !QDjangoWhere();
    return other;
}

/** Returns a QDjangoQuerySet whose elements are ordered using the given keys.
 *
 *  By default the elements will by in ascending order. You can prefix the key
 *  names with a "-" (minus sign) to use descending order.
 *
 * \param keys
 */
template <class T>
QDjangoQuerySet<T> QDjangoQuerySet<T>::orderBy(const QStringList &keys) const
{
    // it is not possible to change ordering once a limit has been set
    Q_ASSERT(!d->lowMark && !d->highMark);

    QDjangoQuerySet<T> other = all();
    other.d->orderBy << keys;
    return other;
}

/** Deletes all objects in the QDjangoQuerySet.
 *
 * \return true if deletion succeeded, false otherwise
 */
template <class T>
bool QDjangoQuerySet<T>::remove()
{
    return d->sqlDelete();
}

/** Returns a QDjangoQuerySet that will automatically "follow" foreign-key
 *  relationships, selecting that additional related-object data when it
 *  executes its query.
 */
template <class T>
QDjangoQuerySet<T> QDjangoQuerySet<T>::selectRelated() const
{
    QDjangoQuerySet<T> other = all();
    other.d->selectRelated = true;
    return other;
}

/** Returns the number of objects in the QDjangoQuerySet, or -1
 *  if the query failed.
 *
 *  If you do not plan to access the objects, you should consider using
 *  count() instead.
 */
template <class T>
int QDjangoQuerySet<T>::size()
{
    if (!d->sqlFetch())
        return -1;
    return d->properties.size();
}

/** Performs an SQL update query for the specified \a fields and returns the
 *  number of rows affected, or -1 if the update failed.
 */
template <class T>
int QDjangoQuerySet<T>::update(const QVariantMap &fields)
{
    return d->sqlUpdate(fields);
}

/** Returns a list of property hashes for the current QDjangoQuerySet.
 *  If no \a fields are specified, all the model's declared fields are returned.
 *
 * \param fields
 */
template <class T>
QList<QVariantMap> QDjangoQuerySet<T>::values(const QStringList &fields)
{
    return d->sqlValues(fields);
}

/** Returns a list of property lists for the current QDjangoQuerySet.
 *  If no \a fields are specified, all the model's fields are returned in the
 *  order they where declared.
 *
 * \param fields
 */
template <class T>
QList<QVariantList> QDjangoQuerySet<T>::valuesList(const QStringList &fields)
{
    return d->sqlValuesList(fields);
}

/** Returns the QDjangoWhere expressing the WHERE clause of the
 * QDjangoQuerySet.
 */
template <class T>
QDjangoWhere QDjangoQuerySet<T>::where() const
{
    return d->resolvedWhere(QDjango::database());
}

/** Assigns the specified queryset to this object.
 *
 * \param other
 */
template <class T>
QDjangoQuerySet<T> &QDjangoQuerySet<T>::operator=(const QDjangoQuerySet<T> &other)
{
    other.d->counter.ref();
    if (!d->counter.deref())
        delete d;
    d = other.d;
    return *this;
}

#endif
