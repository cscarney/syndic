#ifndef FEEDSTORAGEOPERATION_H
#define FEEDSTORAGEOPERATION_H

#include <QObject>
#include <QVector>

#include "feedref.h"

namespace FeedCore {

class StoredItem;

class FeedStorageOperation: public QObject {
    Q_OBJECT

signals:
    void finished();
};

template<typename T>
class FeedStorageQuery : public FeedStorageOperation {
public:
    inline const QVector<T> &result(){ return m_result; }
    inline void setResult(const QVector<T> &&result) { m_result = result; }
    inline void setResult(const T &result ) { m_result = {result}; }
    inline void setResult() { m_result = {}; };

private:
    QVector<T> m_result;
};

typedef FeedStorageQuery<StoredItem> ItemQuery;
typedef FeedStorageQuery<FeedRef> FeedQuery;

}

#endif // FEEDSTORAGEOPERATION_H
