#ifndef STARREDITEMSFEED_H
#define STARREDITEMSFEED_H
#include "feed.h"
namespace FeedCore {
class Context;

/**
 * A Feed implementation which combines only the starred items from
 * all feeds in a context.
 */
class StarredItemsFeed : public Feed
{
public:
    StarredItemsFeed(Context *context, const QString &name, QObject *parent=nullptr);
    Future<ArticleRef> *getArticles(bool unreadFilter) final;
    Updater *updater() final;

private:
    Context *m_context{ nullptr };
    Updater *m_updater { nullptr };
    class StarredUpdater;
};
}

#endif // STARREDITEMSFEED_H
