#ifndef UPDATABLEFEED_H
#define UPDATABLEFEED_H
#include "feed.h"
#include <Syndication/Feed>
namespace FeedCore {
class Updater;

class UpdatableFeed : public Feed
{
public:
    virtual Updater *updater() final;
protected:
    explicit UpdatableFeed(QObject *parent);
private:
    virtual void updateFromSource(const Syndication::FeedPtr &feed) = 0;
    class UpdaterImpl;
    UpdaterImpl *m_updater;
};

}

#endif // UPDATABLEFEED_H
