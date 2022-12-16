#pragma once

#include "feed.h"
#include <QJsonObject>

namespace TTRSS
{
class StorageImpl;
class FeedImpl : public FeedCore::Feed
{
    Q_OBJECT
public:
    FeedImpl(int feedId, StorageImpl *parent);

    FeedCore::Future<FeedCore::ArticleRef> *getArticles(bool unreadFilter) override;
    Feed::Updater *updater() override;
    void updateFromJson(const QJsonObject &feedObj);

private:
    class Updater;
    int m_feedId;
    Updater *m_updater;
};

}
