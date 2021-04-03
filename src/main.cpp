#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQmlContext>
#include <QDir>
#include <QStandardPaths>
#include <KDeclarative/KDeclarative>
#include "context.h"
#include "articlelistmodel.h"
#include "feedlistmodel.h"
#include "qmlarticleref.h"
#include "sqlite/storageimpl.h"
#include "provisionalfeed.h"
#include "updater.h"
#include "iconprovider.h"
using namespace FeedCore;

static QString filePath(QString const &fileName)
{
    QDir appDataDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    if (!appDataDir.mkpath(".")) {
        qDebug("failed to create data dir");
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
    qmlRegisterUncreatableType<Updater>("Updater", 1, 0, "Updater", "abstract base class");
    qmlRegisterUncreatableType<Context>("FeedContext", 1, 0, "FeedContext", "global object");
    qmlRegisterUncreatableType<Enums>("Enums", 1, 0, "Enums", "enum container class");
    qmlRegisterUncreatableType<Feed>("Feed", 1,0, "Feed", "obtained from cpp model");
    qmlRegisterUncreatableType<Article>("Article", 1, 0, "Article", "obtained from cpp model");
    qmlRegisterUncreatableType<QmlArticleRef>("QmlFeedRef", 1, 0, "QmlFeedRef", "obtained from cpp model");
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setOrganizationName("feedkeeper");
    QApplication::setOrganizationDomain("rocksandpaper.com");
    QApplication::setApplicationName("FeedKeeper");
    QQmlApplicationEngine engine;
    QQuickStyle::setStyle("org.kde.desktop");
    KDeclarative::KDeclarative::setupEngine(&engine);
    registerQmlTypes();
    auto *fm = createContext(&app);
    engine.rootContext()->setContextProperty("feedContext", fm);
    engine.addImageProvider("feedicons", new IconProvider(fm));
    engine.load(QUrl("qrc:/qml/main.qml"));
    int result = QApplication::exec();
    return result;
}
