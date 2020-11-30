#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQmlContext>

#ifdef HAVE_KF5
#include <KDeclarative/KDeclarative>
#include <KQuickAddons/QtQuickSettings>
#include <KCrash>
#endif

#include "feedmanager.h"
#include "itemmodel.h"
#include "feeditemmodel.h"
#include "allitemmodel.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setOrganizationName("feedkeeper");
    QApplication::setOrganizationDomain("rocksandpaper.com");
    QApplication::setApplicationName("FeedKeeper");

    QQmlApplicationEngine engine;

#ifdef HAVE_KF5
    QQuickStyle::setStyle("org.kde.desktop");
    KQuickAddons::QtQuickSettings::init();
    KCrash::initialize();
    KDeclarative::KDeclarative decl;
    decl.setDeclarativeEngine(&engine);
    decl.setupContext();
#endif

    auto *fm = new FeedManager(&app);
    qmlRegisterType<FeedManager>("FeedManager", 1, 0, "FeedManager");
    qmlRegisterUncreatableType<ItemModel>("ItemModel", 1, 0, "ItemModel", "abstract base class");
    qmlRegisterType<FeedItemModel>("FeedItemModel", 1, 0, "FeedItemModel");
    qmlRegisterType<AllItemModel>("AllItemModel", 1, 0, "AllItemModel");
    engine.rootContext()->setContextProperty("feedManager", fm);
    engine.load(QUrl("qrc:/qml/main.qml"));

    int result = QApplication::exec();
    return result;
}
