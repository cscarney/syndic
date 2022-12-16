/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "application.h"
#include "cmake-config.h"
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFontDatabase>
#include <QQmlContext>
#include <QStandardPaths>
#include <QWindow>

#ifdef KF5DBusAddons_FOUND
#include <KDBusService>
#endif

#include "articlelistmodel.h"
#include "contentimageitem.h"
#include "contentmodel.h"
#include "context.h"
#include "feedlistmodel.h"
#include "iconprovider.h"
#include "networkaccessmanagerfactory.h"
#include "notificationcontroller.h"
#include "platformhelper.h"
#include "provisionalfeed.h"
#include "qmlarticleref.h"
#include "settings.h"
#include "sqlite/storageimpl.h"
#include "ttrss/storageimpl.h"

struct Application::PrivData {
    FeedCore::Context *context{nullptr};
    Settings settings;
    std::unique_ptr<QQmlApplicationEngine> engine;
    std::unique_ptr<NotificationController> notifier;

#ifdef KF5DBusAddons_FOUND
    KDBusService *service{nullptr};
#endif
};

static QString filePath(QString const &fileName)
{
    QDir appDataDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    if (!appDataDir.mkpath(".")) {
        qWarning() << "failed to create data dir";
        appDataDir = QDir(".");
    }
    return appDataDir.filePath(fileName);
}

static FeedCore::Storage *createStorage()
{
    QString storageName = qEnvironmentVariable("SYNDIC_STORAGE");
    if (storageName == "ttrss") {
        auto *storage = new TTRSS::StorageImpl();
        QString endpoint = qEnvironmentVariable("SYNDIC_TTRSS_ENDPOINT");
        QString username = qEnvironmentVariable("SYNDIC_TTRSS_USERNAME");
        QString password = qEnvironmentVariable("SYNDIC_TTRSS_PASSWORD");
        storage->client().configure(QUrl(endpoint), username, password);
        return storage;
    }
    QString dbPath = filePath("feeds.db");
    return new SqliteStorage::StorageImpl(dbPath);
}

static FeedCore::Context *createContext(QObject *parent = nullptr)
{
    auto *storage = createStorage();
    return new FeedCore::Context(storage, parent);
}

static void registerQmlTypes()
{
    qmlRegisterType<FeedListModel>("com.rocksandpaper.syndic", 1, 0, "FeedListModel");
    qmlRegisterType<ArticleListModel>("com.rocksandpaper.syndic", 1, 0, "ArticleListModel");
    qmlRegisterType<FeedCore::ProvisionalFeed>("com.rocksandpaper.syndic", 1, 0, "ProvisionalFeed");
    qmlRegisterType<ContentModel>("com.rocksandpaper.syndic", 1, 0, "ContentModel");
    qmlRegisterType<ContentImageItem>("com.rocksandpaper.syndic", 1, 0, "ContentImage");
    qmlRegisterUncreatableType<FeedCore::Feed::Updater>("com.rocksandpaper.syndic", 1, 0, "Updater", "abstract base class");
    qmlRegisterUncreatableType<FeedCore::Context>("com.rocksandpaper.syndic", 1, 0, "FeedContext", "global object");
    qmlRegisterUncreatableType<FeedCore::Feed>("com.rocksandpaper.syndic", 1, 0, "Feed", "obtained from cpp model");
    qmlRegisterUncreatableType<FeedCore::Article>("com.rocksandpaper.syndic", 1, 0, "Article", "obtained from cpp model");
    qmlRegisterUncreatableType<QmlArticleRef>("com.rocksandpaper.syndic", 1, 0, "QmlFeedRef", "obtained from cpp model");
    qmlRegisterUncreatableType<PlatformHelper>("com.rocksandpaper.syndic", 1, 0, "PlatformHelper", "global object");
    qmlRegisterUncreatableType<ContentBlock>("com.rocksandpaper.syndic", 1, 0, "ContentBlock", "obtained from contentmodel");
    qmlRegisterUncreatableType<ImageBlock>("com.rocksandpaper.syndic", 1, 0, "ImageBlock", "obtained from contentmodel");
    qmlRegisterUncreatableType<TextBlock>("com.rocksandpaper.syndic", 1, 0, "TextBlock", "obtained from contentmodel");
}

static void enableSettingsAutosave(const Settings &settings)
{
    const QMetaObject *metaObject = settings.metaObject();
    int propCount = metaObject->propertyCount();
    const QMetaMethod saveSlot = metaObject->method(metaObject->indexOfSlot("save()"));
    for (int i = 0; i < propCount; ++i) {
        const QMetaProperty &prop = metaObject->property(i);
        if (prop.hasNotifySignal()) {
            const QMetaMethod &signal = prop.notifySignal();
            QObject::connect(&settings, signal, &settings, saveSlot, Qt::QueuedConnection);
        }
    }
}

#ifdef ANDROID
// https://bugreports.qt.io/browse/QTBUG-69494
static void loadEmbeddedFonts()
{
    QDirIterator it("/system/fonts", QDir::NoFilter, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        QString path{it.filePath()};
        if (path.endsWith("_subset.ttf")) {
            continue;
        }
        QFontDatabase::addApplicationFont(path);
    }
    QFont::insertSubstitution("sans", "Roboto");
    QFont::insertSubstitution("serif", "Noto Serif");
}
#endif

Application::Application(int &argc, char **argv)
    : SyndicApplicationBase(argc, argv)
    , d{std::make_unique<PrivData>()}
{
    setOrganizationName("syndic");
    setOrganizationDomain("rocksandpaper.com");
    setApplicationName("syndic");
    setDesktopFileName("com.rocksandpaper.syndic.desktop");
    setApplicationDisplayName(tr("Syndic"));
    setWindowIcon(QIcon::fromTheme("com.rocksandpaper.syndic"));

#ifdef KF5DBusAddons_FOUND
    d->service = new KDBusService(KDBusService::Unique, this);
    QObject::connect(d->service, &KDBusService::activateRequested, this, &Application::onActivateRequested);
#endif
    registerQmlTypes();
    setQuitOnLastWindowClosed(false);
    QObject::connect(this, &QGuiApplication::lastWindowClosed, this, &Application::onLastWindowClosed);
    QQmlEngine::setObjectOwnership(settings(), QQmlEngine::CppOwnership);
    enableSettingsAutosave(d->settings);

#ifdef ANDROID
    loadEmbeddedFonts();
#endif

    d->context = createContext(this);
    bindContextPropertiesToSettings();
}

Application::~Application() = default;

FeedCore::Context *Application::context()
{
    return d->context;
}

Settings *Application::settings()
{
    return &d->settings;
}

void Application::loadMainWindow()
{
    if (!d->engine) {
        d->engine = std::make_unique<QQmlApplicationEngine>();
        d->engine->setNetworkAccessManagerFactory(new NetworkAccessManagerFactory);
        d->engine->rootContext()->setContextProperty("feedContext", d->context);
        d->engine->rootContext()->setContextProperty("platformHelper", new PlatformHelper);
        d->engine->rootContext()->setContextProperty("globalSettings", settings());
        d->engine->addImageProvider("feedicons", new IconProvider);
        d->engine->load(QUrl("qrc:/qml/main.qml"));
        d->notifier = nullptr;
    }
    activateMainWindow();
}

void Application::activateMainWindow()
{
    const auto rootObjects = d->engine->rootObjects();
    for (QObject *object : rootObjects) {
        QWindow *window = qobject_cast<QWindow *>(object);
        if (window != nullptr) {
            window->setVisible(true);
            window->requestActivate();
            return;
        }
    };
}

void Application::unloadEngine()
{
    const auto rootObjects = d->engine->rootObjects();
    for (auto *object : rootObjects) {
        delete object;
    }
    d->engine->clearComponentCache();
    d->engine->collectGarbage();
    d->engine = nullptr;
}

void Application::onLastWindowClosed()
{
#ifdef ANDROID
    quit();
#else
    d->settings.save();
    const QWindowList windows = topLevelWindows();
    if (windows.size() > 1) {
        // we sometimes get called when dialogs are closed
        // possibly related: https://bugreports.qt.io/browse/QTBUG-80483
        for (auto *eachWindow : windows) {
            if (eachWindow->visibility() != QWindow::Hidden) {
                return;
            }
        }
    }
    if (!d->settings.runInBackground()) {
        quit();
        return;
    }
    unloadEngine();
    d->notifier = std::make_unique<NotificationController>(d->context);
    QObject::connect(d->notifier.get(), &NotificationController::activate, this, &Application::loadMainWindow);
#endif
}

void Application::onActivateRequested(const QStringList & /*unused*/, const QString & /*unused*/)
{
    loadMainWindow();
}

void Application::bindContextPropertiesToSettings()
{
    syncDefaultUpdateInterval();
    QObject::connect(settings(), &Settings::updateIntervalChanged, this, &Application::syncDefaultUpdateInterval);

    syncAutomaticUpdates();
    QObject::connect(settings(), &Settings::automaticUpdatesChanged, this, &Application::syncAutomaticUpdates);

    syncExpireAge();
    QObject::connect(settings(), &Settings::expireItemsChanged, this, &Application::syncExpireAge);
    QObject::connect(settings(), &Settings::expireAgeChanged, this, &Application::syncExpireAge);

    if (d->settings.updateOnStart()) {
        QObject::connect(d->context, &FeedCore::Context::feedListPopulated, d->context, &FeedCore::Context::requestUpdate);
    }
}

void Application::syncAutomaticUpdates()
{
    d->context->setDefaultUpdateEnabled(d->settings.automaticUpdates());
}

void Application::syncDefaultUpdateInterval()
{
    d->context->setDefaultUpdateInterval(d->settings.updateInterval());
}

void Application::syncExpireAge()
{
    d->context->setExpireAge(d->settings.expireItems() ? d->settings.expireAge() : 0);
}
