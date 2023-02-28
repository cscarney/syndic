#ifndef FEEDCORE_CATEGORYFEED_H
#define FEEDCORE_CATEGORYFEED_H

#include "aggregatefeed.h"

namespace FeedCore
{
class Context;

class CategoryFeed : public FeedCore::AggregateFeed
{
public:
    CategoryFeed(FeedCore::Context *ctx, const QString &category, QObject *parent = nullptr);
};

} // namespace FeedCore

#endif // FEEDCORE_CATEGORYFEED_H
