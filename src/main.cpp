#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQmlContext>

#include <KDeclarative/KDeclarative>

#include "context.h"
#include "feeditemmodel.h"
#include "allitemmodel.h"
#include "feedlistmodel.h"
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

    auto *fm = new Context(&app);

    qmlRegisterType<Context>("FeedManager", 1, 0, "FeedManager");
    engine.rootContext()->setContextProperty("feedManager", fm);

    qmlRegisterUncreatableType<Enums>("Enums", 1, 0, "Enums", "enum container class");
    qmlRegisterUncreatableType<FeedRef>("FeedRef", 1,0,"FeedRef", "obtained from cpp model");
    qmlRegisterUncreatableType<QmlFeedRef>("QmlFeedRef", 1, 0, "QmlFeedRef", "obtained from cpp model");
    QMetaType::registerConverter<QmlFeedRef, FeedRef>();

    qmlRegisterType<FeedListModel>("FeedListModel", 1, 0, "FeedListModel");
    qmlRegisterType<FeedItemModel>("FeedItemModel", 1, 0, "FeedItemModel");
    qmlRegisterType<AllItemModel>("AllItemModel", 1, 0, "AllItemModel");

    engine.load(QUrl("qrc:/qml/main.qml"));

    int result = QApplication::exec();
    return result;
}
