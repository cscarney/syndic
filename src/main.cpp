#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQmlContext>

#include <KDeclarative/KDeclarative>

#include "context.h"
#include "feeditemmodel.h"
#include "feedlistmodel.h"
#include "qmlarticleref.h"
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

    auto *fm = new FeedCore::Context(&app);
    qmlRegisterType<FeedListModel>("FeedListModel", 1, 0, "FeedListModel");
    qmlRegisterType<FeedItemModel>("FeedItemModel", 1, 0, "FeedItemModel");
    qmlRegisterType<Context>("FeedManager", 1, 0, "FeedManager");
    qmlRegisterUncreatableType<Enums>("Enums", 1, 0, "Enums", "enum container class");
    qmlRegisterUncreatableType<FeedRef>("FeedRef", 1,0,"FeedRef", "obtained from cpp model");
    qmlRegisterUncreatableType<QmlFeedRef>("QmlFeedRef", 1, 0, "QmlFeedRef", "obtained from cpp model");
    qmlRegisterUncreatableType<Article>("Article", 1, 0, "Article", "obtained from cpp model");
    qmlRegisterUncreatableType<ArticleRef>("FeedRef", 1,0,"FeedRef", "obtained from cpp model");
    qmlRegisterUncreatableType<QmlArticleRef>("QmlFeedRef", 1, 0, "QmlFeedRef", "obtained from cpp model");
    QMetaType::registerConverter<QmlFeedRef, FeedRef>();
    QMetaType::registerConverter<QmlArticleRef, ArticleRef>();
    engine.rootContext()->setContextProperty("feedManager", fm);
    engine.load(QUrl("qrc:/qml/main.qml"));
    int result = QApplication::exec();
    return result;
}
