#include "feed.h"
#include "updatablefeed.h"
#include <QMap>
#include <QPluginLoader>
#include <QString>

namespace FeedCore
{

// An interface for plugin factory classes
class UpdaterPlugin
{
public:
    virtual ~UpdaterPlugin() = default;
    // Create an updater instance for a given feed
    virtual Feed::Updater *createUpdater(UpdatableFeed *feed) = 0;
};

// A class that creates updater instances using plugins
class UpdaterFactory : QObject
{
    Q_OBJECT
public:
    static UpdaterFactory *instance();

    // Create an updater instance for a given feed
    Feed::Updater *createUpdater(UpdatableFeed *feed);

private:
    explicit UpdaterFactory(QObject *parent = nullptr);
    QMap<QString, QPluginLoader *> m_loaders;
};

} // namespace FeedCore

Q_DECLARE_INTERFACE(FeedCore::UpdaterPlugin, "com.rocksandpaper.syndic.UpdaterPlugin")
