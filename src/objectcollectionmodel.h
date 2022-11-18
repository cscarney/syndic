/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef OBJECTCOLLECTIONMODEL_H
#define OBJECTCOLLECTIONMODEL_H

#include <QAbstractListModel>
#include <QMetaProperty>
#include <set>
#include <vector>

class ObjectCollectionModelBase : public QAbstractListModel
{
    Q_OBJECT
protected:
    virtual void onListItemPropertyChanged() = 0;
};

template<typename T>
class ObjectCollectionModel : public ObjectCollectionModelBase
{
    typedef typename std::remove_reference<decltype(*std::declval<T>())>::type QObjectType;
    static_assert(std::is_base_of<QObject, QObjectType>::value, "ObjectCollectionModel type must refer to a QObject-derived class");

public:
    ObjectCollectionModel();

    void add(const T &item);

    void remove(const T &item);

    QModelIndex index(int row, int column, const QModelIndex &parent) const override;

    int rowCount(const QModelIndex &parent) const override;

    QVariant data(const QModelIndex &index, int role) const override;

private:
    std::vector<T> m_items;
    std::unordered_map<QObject *, QPersistentModelIndex> m_object_indices;
    mutable std::unordered_map<int, QMetaProperty> m_role_map;

    void onListItemPropertyChanged() final;
};

template<typename T>
ObjectCollectionModel<T>::ObjectCollectionModel()
{
    const auto roles = roleNames();
    for (auto i = roles.keyValueBegin(); i != roles.keyValueEnd(); ++i) {
        const QMetaObject &metaObject = QObjectType::staticMetaObject;
        m_role_map[i->first] = metaObject.property(metaObject.indexOfProperty(i->second));
    }
}

template<typename T>
void ObjectCollectionModel<T>::add(const T &item)
{
    QMetaMethod propertyChangeHandler = metaObject()->method(metaObject()->indexOfMethod("onListItemPropertyChanged()"));
    int i = m_items.size;
    beginInsertRows(QModelIndex(), i, i);
    m_items << item;
    endInsertRows();
    m_object_indices[item] = index(i, 0, QModelIndex());
    for (auto &pair : m_role_map) {
        QObject::connect(item, pair.second.notifySignal(), this, propertyChangeHandler, Qt::QueuedConnection);
    }
}

template<typename T>
void ObjectCollectionModel<T>::remove(const T &item)
{
    for (auto i = m_items.begin(); i != m_items.end();) {
        if (*i == item) {
            int index = i - m_items.begin();
            beginRemoveRows(QModelIndex(), index, index);
            i = m_items.erase(i);
            endRemoveRows();
        } else {
            ++i;
        }
    }
}

template<typename T>
QModelIndex ObjectCollectionModel<T>::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return QModelIndex();
    }
    return createIndex(row, column, &*m_items[row]);
}

template<typename T>
int ObjectCollectionModel<T>::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_items.size();
}

template<typename T>
QVariant ObjectCollectionModel<T>::data(const QModelIndex &index, int role) const
{
    QObject *target = static_cast<QObjectType *>(index.internalPointer());
    QMetaProperty &prop = m_role_map[role];
    if (Q_UNLIKELY(target == nullptr || !prop.isValid())) {
        return QVariant();
    }
    return prop.read(target);
}

template<typename T>
void ObjectCollectionModel<T>::onListItemPropertyChanged()
{
    QObject *sender = QObject::sender();
    QPersistentModelIndex &i = m_object_indices[sender];
    if (!i.isValid()) {
        int row = std::find(m_items.cbegin(), m_items.cend(), sender) - m_items.cbegin();
        if (row >= m_items.length) {
            qWarning("Failed to find updated item in list!");
            return;
        }
        i = index(row, 0, QModelIndex());
    }
    emit dataChanged(i, i);
}

#endif // OBJECTCOLLECTIONMODEL_H
