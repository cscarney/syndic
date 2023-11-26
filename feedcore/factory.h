/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include <QHash>
#include <QPointer>
#include <QWeakPointer>

namespace FeedCore
{
template<typename KeyType, typename ValueType, typename StorageType, typename PointerType>
class Factory
{
public:
    template<typename... Args>
    PointerType getInstance(KeyType key, Args &&...args)
    {
        auto &instance = m_instances[key];
        if (instance.isNull()) {
            auto newArticle = PointerType(new ValueType(key, std::forward<Args>(args)...));
            instance = newArticle;
            return newArticle;
        }
        return instance;
    }

    bool hasInstance(KeyType key) const
    {
        return !m_instances[key].isNull();
    }

private:
    QHash<KeyType, StorageType> m_instances;
};

template<typename KeyType, typename ValueType>
using SharedFactory = Factory<KeyType, ValueType, QWeakPointer<ValueType>, QSharedPointer<ValueType>>;

template<typename KeyType, typename ValueType>
using ObjectFactory = Factory<KeyType, ValueType, QPointer<ValueType>, ValueType *>;

}
