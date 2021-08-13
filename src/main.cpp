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

#ifdef KF5Declarative_FOUND
#include <KDeclarative/KDeclarative>
#endif

#include "context.h"
#include "articlelistmodel.h"
#include "feedlistmodel.h"
#include "qmlarticleref.h"
#include "sqlite/storageimpl.h"
#include "provisionalfeed.h"
#include "updater.h"
#include "iconprovider.h"
#include "newitemnotifier.h"
#include "platformhelper.h"
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
    qmlRegisterUncreatableType<Updater>("Updater", 1, 0, "Updater", "abstract base class");
    qmlRegisterUncreatableType<Context>("FeedContext", 1, 0, "FeedContext", "global object");
    qmlRegisterUncreatableType<Feed>("Feed", 1,0, "Feed", "obtained from cpp model");
    qmlRegisterUncreatableType<Article>("Article", 1, 0, "Article", "obtained from cpp model");
    qmlRegisterUncreatableType<QmlArticleRef>("QmlFeedRef", 1, 0, "QmlFeedRef", "obtained from cpp model");
    qmlRegisterUncreatableType<PlatformHelper>("PlatformHelper", 1, 0, "PlatformHelper", "global object");
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
    engine.addImageProvider("feedicons", new IconProvider(fm));
    engine.load(QUrl("qrc:/qml/main.qml"));
    int result = QApplication::exec();
    return result;
}
