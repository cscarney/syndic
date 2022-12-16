#include "ttrss/feedimpl.h"
#include "ttrss/storageimpl.h"

using namespace FeedCore;
using namespace TTRSS;

class FeedImpl::Updater : public Feed::Updater
{
public:
    using Feed::Updater::Updater;

private:
    void run() override
    {
        finish();
    };
};

FeedImpl::FeedImpl(int feedId, StorageImpl *parent)
    : Feed{parent}
    , m_feedId{feedId}
    , m_updater{new Updater(this, this)}
{
}

Future<ArticleRef> *FeedImpl::getArticles(bool unreadFilter)
{
    if (auto *storage = qobject_cast<StorageImpl *>(parent())) {
        return storage->getArticles(m_feedId, unreadFilter ? "unread" : "all_articles", 0);
    }
    Q_UNREACHABLE();
}

Feed::Updater *FeedImpl::updater()
{
    return m_updater;
}

void FeedImpl::updateFromJson(const QJsonObject &feedObj)
{
    setName(feedObj["title"].toString());
    setUnreadCount(feedObj["unread"].toInt());
}
