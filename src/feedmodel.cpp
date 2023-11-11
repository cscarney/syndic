#include "feedmodel.h"
#include "article.h"

using namespace FeedCore;

struct FeedModel::PrivData {
    Feed *feed{nullptr};
};

FeedModel::FeedModel(QObject *parent)
    : ArticleListModel{parent}
    , d{std::make_unique<PrivData>()}
{
}

FeedModel::~FeedModel() = default;

Feed *FeedModel::feed() const
{
    return d->feed;
}

void FeedModel::setFeed(Feed *feed)
{
    if (d->feed != feed) {
        if (d->feed) {
            QObject::disconnect(d->feed, nullptr, this, nullptr);
        }
        d->feed = feed;
        if (active()) {
            clear();
            init(); // re-run init() to re-connect to the new feed
            refresh();
        }
        emit feedChanged();
    }
}

void FeedModel::init()
{
    QObject::connect(d->feed, &Feed::articleAdded, this, &FeedModel::addItem);
    QObject::connect(d->feed, &Feed::statusChanged, this, &FeedModel::onStatusChanged);
    QObject::connect(d->feed, &Feed::reset, this, &FeedModel::refresh);
}

QFuture<ArticleRef> FeedModel::getArticles()
{
    if (d->feed) {
        return d->feed->getArticles(unreadFilter());
    }
    return QFuture<ArticleRef>();
}

void FeedModel::setStatusFromUpstream()
{
    auto *feed = d->feed;
    if (feed != nullptr) {
        setStatus(feed->status());
    }
}

static bool compareDatesDescending(const ArticleRef &l, const ArticleRef &r)
{
    return l->date() > r->date();
}

ArticleListModel::ArticleComparator FeedModel::getArticleComparator()
{
    return &compareDatesDescending;
}

void FeedModel::requestUpdate()
{
    feed()->updater()->start();
    removeRead();
}

void FeedModel::onStatusChanged()
{
    if (status() != Feed::Loading) {
        setStatus(d->feed->status());
    }
}
