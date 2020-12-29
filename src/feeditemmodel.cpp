#include "feeditemmodel.h"
#include <QDebug>
#include "updater.h"
#include "feed.h"
using namespace FeedCore;

FeedItemModel::FeedItemModel(QObject * parent):
    ItemModel(parent)
{

}

FeedRef FeedItemModel::feed() const
{
    return m_feed;
}

void FeedItemModel::setFeed(const FeedRef &feed)
{
    if (m_feed != feed) {
        m_feed = feed;
        if (active()) {
            qDebug() << "set feed after initialization!!";
        }
        emit feedChanged();
    }
}

void FeedItemModel::initialize() {
    QObject::connect(m_feed.get(), &Feed::itemAdded, this, &FeedItemModel::onItemAdded);
    QObject::connect(m_feed.get(), &Feed::statusChanged, this, &FeedItemModel::onStatusChanged);
    refresh();
}


void FeedItemModel::requestUpdate()
{
    feed()->updater()->start();
}

Future<ArticleRef> *FeedItemModel::startQuery()
{
    if (m_feed.isNull()) {
        qDebug() << "starting query with no feed set!";
        return nullptr;
    }
    return m_feed->startItemQuery(unreadFilter());
}

void FeedItemModel::setStatusFromUpstream()
{
    setStatus(m_feed->status());
}

void FeedItemModel::onStatusChanged()
{
    if (status() != Enums::Loading) {
        setStatus(m_feed->status());
    }
}
