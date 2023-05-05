#include "testplugin.h"
#include <QDebug>
#include <QTimer>

namespace
{
class UpdaterImpl : public FeedCore::Feed::Updater
{
public:
    UpdaterImpl(FeedCore::UpdatableFeed *feed)
        : FeedCore::Feed::Updater(feed, feed)
    {
    }

private:
    void run() override;
};
}

void UpdaterImpl::run()
{
    qDebug() << "hello";
    QTimer::singleShot(0, this, &UpdaterImpl::finish);
}

FeedCore::Feed::Updater *TestPlugin::createUpdater(FeedCore::UpdatableFeed *feed)
{
    return new UpdaterImpl(feed);
}
