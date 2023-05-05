#pragma once
#include "feed.h"

namespace FeedCore
{
class UpdatableFeed;

class DefaultUpdater : public Feed::Updater
{
public:
    DefaultUpdater(UpdatableFeed *feed, QObject *parent);
    void run() final;
    void abort() final;

private:
    UpdatableFeed *m_updatableFeed{nullptr};
    class LoadOperation;
    class PreloadQueue;
    QPointer<LoadOperation> m_operation;
    QPointer<PreloadQueue> m_preloadQueue;

    void onSucceeded(const Syndication::FeedPtr &feed, const QUrl &changeUrl);
    void onFailed(const QString &errorString);
};
}
