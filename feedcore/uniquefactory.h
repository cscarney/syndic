#ifndef UNIQUEFACTORY_H
#define UNIQUEFACTORY_H
#include <QHash>
#include <QWeakPointer>

namespace FeedCore {
template<typename KeyType, typename ValueType>
class UniqueFactory
{
public:
    bool hasInstance(KeyType key) {
        return m_instances.contains(key) && !m_instances.value(key).isNull();
    }

    template<typename... Args>
    QSharedPointer<ValueType> getInstance(KeyType key, Args&&... args) {
        auto &instance = m_instances[key];
        if (instance.isNull()) {
            auto newArticle = QSharedPointer<ValueType>(new ValueType(key, std::forward<Args>(args)...));
            instance = newArticle;
            return newArticle;
        }
        return instance.toStrongRef();
    }

private:
    QHash<KeyType, QWeakPointer<ValueType>> m_instances;
};
}
#endif // UNIQUEFACTORY_H
