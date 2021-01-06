#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQmlContext>
#include <KDeclarative/KDeclarative>
#include "context.h"
#include "articlelistmodel.h"
#include "feedlistmodel.h"
#include "qmlarticleref.h"
#include "qmlfeedref.h"
#include "sqlite/storageimpl.h"
using namespace FeedCore;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setOrganizationName("feedkeeper");
    QApplication::setOrganizationDomain("rocksandpaper.com");
    QApplication::setApplicationName("FeedKeeper");

    QQmlApplicationEngine engine;

    QQuickStyle::setStyle("org.kde.desktop");
    KDeclarative::KDeclarative::setupEngine(&engine);

    qmlRegisterType<FeedListModel>("FeedListModel", 1, 0, "FeedListModel");
    qmlRegisterType<ArticleListModel>("ArticleListModel", 1, 0, "ArticleListModel");
    qmlRegisterUncreatableType<Context>("FeedContext", 1, 0, "FeedContext", "global object");
    qmlRegisterUncreatableType<Enums>("Enums", 1, 0, "Enums", "enum container class");
    qmlRegisterUncreatableType<Feed>("Feed", 1,0, "Feed", "obtained from cpp model");
    qmlRegisterUncreatableType<FeedRef>("FeedRef", 1,0,"FeedRef", "obtained from cpp model");
    qmlRegisterUncreatableType<QmlFeedRef>("QmlFeedRef", 1, 0, "QmlFeedRef", "obtained from cpp model");
    qmlRegisterUncreatableType<Article>("Article", 1, 0, "Article", "obtained from cpp model");
    qmlRegisterUncreatableType<ArticleRef>("ArticleRef", 1,0,"ArticleRef", "obtained from cpp model");
    qmlRegisterUncreatableType<QmlArticleRef>("QmlFeedRef", 1, 0, "QmlFeedRef", "obtained from cpp model");
    QMetaType::registerConverter<QmlFeedRef, FeedRef>();
    QMetaType::registerConverter<QmlArticleRef, ArticleRef>();
    auto *fm = new FeedCore::Context(new Sqlite::StorageImpl, &app);
    engine.rootContext()->setContextProperty("feedContext", fm);
    engine.load(QUrl("qrc:/qml/main.qml"));
    int result = QApplication::exec();
    return result;
}
