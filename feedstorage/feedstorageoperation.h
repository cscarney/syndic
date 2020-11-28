#ifndef FEEDSTORAGEOPERATION_H
#define FEEDSTORAGEOPERATION_H

#include <QObject>
#include <QVector>

#include "storeditem.h"
#include "storedfeed.h"

class FeedStorageOperation: public QObject {
    Q_OBJECT

signals:
    void finished();
};

template<typename T>
class FeedStorageQuery : public FeedStorageOperation {
public:
    QVector<T> result;
};

typedef FeedStorageQuery<StoredItem> ItemQuery;
typedef FeedStorageQuery<StoredFeed> FeedQuery;


#endif // FEEDSTORAGEOPERATION_H
