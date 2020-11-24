#ifndef STOREDITEM_H
#define STOREDITEM_H
#include "feeditemheaders.h"
#include "feeditemstatus.h"

struct StoredItem {
    /* unique id within the storage backend */
    qint64 id;

    /* id of the feed the item belongs to, within the storage backend */
    qint64 feedId;

    /* identifier provided by the feed, unique within the feed */
    QString localId;

    /* metadata provided by the feed */
    FeedItemHeaders headers;

    /* content provided by the feed */
    QString content;

    /* status flags, set by the client */
    FeedItemStatus status;
};

#endif // STOREDITEM_H
