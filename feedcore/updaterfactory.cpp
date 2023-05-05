#include "updaterfactory.h"
#include "defaultupdater.h"
#include <QCoreApplication>
#include <QDir>
#include <QJsonArray>

FeedCore::UpdaterFactory::UpdaterFactory(QObject *parent)
    : QObject(parent)
{
    const auto libPaths = QCoreApplication::libraryPaths();
    for (const auto &libPath : libPaths) {
        QDir pluginDir(libPath + "/updater-plugins");
        const auto files = pluginDir.entryList(QDir::Files);
        for (const auto &file : files) {
            auto *loader = new QPluginLoader(pluginDir.absoluteFilePath(file), this);
            const auto metadata = loader->metaData()["MetaData"].toObject();
            const auto schemes = metadata["schemes"].toArray();
            for (const auto &scheme : schemes) {
                if (scheme.isString()) {
                    m_loaders.insert(scheme.toString(), loader);
                }
            }
        }
    }
}

FeedCore::UpdaterFactory *FeedCore::UpdaterFactory::instance()
{
    static auto *instance = new UpdaterFactory;
    return instance;
}

FeedCore::Feed::Updater *FeedCore::UpdaterFactory::createUpdater(UpdatableFeed *feed)
{
    const QString scheme = feed->url().scheme();
    if (m_loaders.contains(scheme)) {
        QPluginLoader *loader = m_loaders.value(scheme);
        UpdaterPlugin *plugin = qobject_cast<UpdaterPlugin *>(loader->instance());
        if (plugin) {
            return plugin->createUpdater(feed);
        }
    }
    return new DefaultUpdater(feed, feed);
}
