#pragma once
#include "updaterfactory.h"

class TestPlugin : public QObject, public FeedCore::UpdaterPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.rocksandpaper.syndic.UpdaterPlugin" FILE "TestPlugin.json")
    Q_INTERFACES(FeedCore::UpdaterPlugin)
public:
    FeedCore::Feed::Updater *createUpdater(FeedCore::UpdatableFeed *feed) override;
};
