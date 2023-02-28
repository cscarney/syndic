#include "categoryfeed.h"
#include "context.h"

namespace FeedCore
{

CategoryFeed::CategoryFeed(FeedCore::Context *ctx, const QString &category, QObject *parent)
    : AggregateFeed(parent)
{
    setName(category);
    const QSet<Feed *> categories = ctx->getCategoryFeeds(category);
    for (auto *f : categories) {
        addFeed(f);
    }
}

} // namespace FeedCore
