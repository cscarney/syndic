/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "cmake-config.h"
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQmlContext>
#include <QDir>
#include <QDirIterator>
#include <QFontDatabase>
#include <QStandardPaths>
#include <QWindow>

#ifdef KF5Declarative_FOUND
#include <KDeclarative/KDeclarative>
#endif

#ifdef KF5DBusAddons_FOUND
#include <KDBusService>
#endif

#include "context.h"
#include "articlelistmodel.h"
#include "feedlistmodel.h"
#include "qmlarticleref.h"
#include "sqlite/storageimpl.h"
#include "provisionalfeed.h"
#include "iconprovider.h"
#include "newitemnotifier.h"
#include "platformhelper.h"
#include "contentmodel.h"
#include "contentimageitem.h"
using namespace FeedCore;

static QString filePath(QString const &fileName)
{
    QDir appDataDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    if (!appDataDir.mkpath(".")) {
        qWarning("failed to create data dir");
        appDataDir = QDir(".");
    }
    return appDataDir.filePath(fileName);
}

static FeedCore::Context *createContext(QObject *parent = nullptr) {
    QString dbPath = filePath("feeds.db");
    auto *fm = new Sqlite::StorageImpl(dbPath);  // ownership passes to context
    return new FeedCore::Context(fm, parent);
}

static void registerQmlTypes() {
    qmlRegisterType<FeedListModel>("FeedListModel", 1, 0, "FeedListModel");
    qmlRegisterType<ArticleListModel>("ArticleListModel", 1, 0, "ArticleListModel");
    qmlRegisterType<ProvisionalFeed>("ProvisionalFeed", 1, 0, "ProvisionalFeed");
    qmlRegisterType<NewItemNotifier>("NewItemNotifier", 1, 0, "NewItemNotifier");
    qmlRegisterType<ContentModel>("ContentModel", 1, 0, "ContentModel");
    qmlRegisterType<ContentImageItem>("ContentImage", 1, 0, "ContentImage");
    qmlRegisterUncreatableType<Feed::Updater>("Updater", 1, 0, "Updater", "abstract base class");
    qmlRegisterUncreatableType<Context>("FeedContext", 1, 0, "FeedContext", "global object");
    qmlRegisterUncreatableType<Feed>("Feed", 1,0, "Feed", "obtained from cpp model");
    qmlRegisterUncreatableType<Article>("Article", 1, 0, "Article", "obtained from cpp model");
    qmlRegisterUncreatableType<QmlArticleRef>("QmlFeedRef", 1, 0, "QmlFeedRef", "obtained from cpp model");
    qmlRegisterUncreatableType<PlatformHelper>("PlatformHelper", 1, 0, "PlatformHelper", "global object");
    qmlRegisterUncreatableType<ContentBlock>("ContentBlock", 1, 0, "ContentBlock", "obtained from contentmodel");
    qmlRegisterUncreatableType<ImageBlock>("ImageBlock", 1, 0, "ImageBlock", "obtained from contentmodel");
    qmlRegisterUncreatableType<TextBlock>("TextBlock", 1, 0, "TextBlock", "obtained from contentmodel");
}

#ifdef ANDROID
// https://bugreports.qt.io/browse/QTBUG-69494
static void loadEmbeddedFonts(const QGuiApplication &app) {
    QDirIterator it("/system/fonts", QDir::NoFilter, QDirIterator::Subdirectories);
    while(it.hasNext()) {
        it.next();
        QString path { it.filePath() };
        if (path.endsWith("_subset.ttf")) {
            continue;
        }
        QFontDatabase::addApplicationFont(path);
    }
    QFont::insertSubstitution("sans", "Roboto");
    QFont::insertSubstitution("serif", "Noto Serif");
}
#endif

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setOrganizationName("feedkeeper");
    QApplication::setOrganizationDomain("rocksandpaper.com");
    QApplication::setApplicationName("FeedKeeper");
    QQmlApplicationEngine engine;

#ifdef KF5DBusAddons_FOUND
    KDBusService dbusService(KDBusService::Unique, &engine);
    QObject::connect(&dbusService, &KDBusService::activateRequested, &engine, [&engine]{
        const auto rootObjects = engine.rootObjects();
        for (QObject *object : rootObjects) {
            QWindow *window = qobject_cast<QWindow*>(object);
            if (window) {
                window->setVisible(true);
                window->requestActivate();
                return;
            }
        };
    });
#endif

#ifdef ANDROID
    QQuickStyle::setStyle("Material");
    loadEmbeddedFonts(app);
#else
    QQuickStyle::setStyle("org.kde.desktop");
#endif

#ifdef KF5Declarative_FOUND
    KDeclarative::KDeclarative::setupEngine(&engine);
#endif

    registerQmlTypes();
    auto *fm = createContext(&app);
    engine.rootContext()->setContextProperty("feedContext", fm);
    engine.rootContext()->setContextProperty("platformHelper", new PlatformHelper);
    engine.addImageProvider("feedicons", new IconProvider);
    engine.load(QUrl("qrc:/qml/main.qml"));
    int result = QApplication::exec();
    return result;
}
