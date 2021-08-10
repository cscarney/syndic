#include "starreditemsfeed.h"
#include "context.h"
#include "updater.h"
using namespace FeedCore;

class StarredItemsFeed::StarredUpdater : public Updater {
public:
    explicit StarredUpdater(StarredItemsFeed *parent) :
        Updater(parent, parent)
    {}

    void run() override {
        finish();
    }
    void abort() override {}
};

StarredItemsFeed::StarredItemsFeed(FeedCore::Context *context, const QString &name, QObject *parent) :
    Feed(parent),
    m_context { context },
    m_updater { new  StarredUpdater(this) }
{
    setName(name);
}

Future<ArticleRef> *StarredItemsFeed::getArticles(bool unreadFilter)
{
    return m_context->getStarred();
}

Updater *StarredItemsFeed::updater()
{
    return m_updater;
}
