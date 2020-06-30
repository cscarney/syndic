#ifndef STOREDITEM_H
#define STOREDITEM_H
#include "feeditemheaders.h"
#include "feeditemstatus.h"

struct StoredItem {
    qint64 id;
    qint64 feedId;
    QString localId;
    FeedItemHeaders headers;
    QString content;
    FeedItemStatus status;
};

#endif // STOREDITEM_H
