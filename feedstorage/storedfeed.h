#ifndef STOREDFEED_H
#define STOREDFEED_H
#include "feedheaders.h"

struct StoredFeed {
    qint64 id;
    qint64 sourceId;
    QString localId;
    FeedHeaders headers;
};

#endif // STOREDFEED_H
