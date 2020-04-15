import QtQuick 2.12
import Qt.labs.settings 1.1
import QtQuick.Controls 2.12
import org.kde.kirigami 2.7 as Kirigami

Kirigami.ApplicationWindow {
    id: mainWindow

    Settings {
        category: "Window"
        property alias width: mainWindow.width
        property alias height: mainWindow.height
    }

    property real itemListProportion:  0.38
    property real feedListProportion: 0.23

    property bool isFirstPage: pageStack.firstVisibleItem === pageStack.items[0]
    property string pageTitle: pageStack.firstVisibleItem ? pageStack.firstVisibleItem.title : ""
    title: (pageTitle.length>0 ? pageTitle+" - " : "") + qsTr("FeedKeeper")

    globalDrawer: FeedListDrawer {
        property real actualWidth: drawerOpen && !modal ? width : 0
        width: feedListProportion * mainWindow.width
        modal: true
        onFeedSelected: currentItem.isSpecial ? pushAll() : pushFeed(currentItem)

        actions: [
            Kirigami.Action {
                // @disable-check M16
                separator: true
            },
            Kirigami.Action {
                text: "Add Content..."
                iconName: "list-add"
            },
            Kirigami.Action {
                text: "Preferences"
                iconName: "settings-configure"
            },
            Kirigami.Action {
                text: "About FeedKeeper"
                iconName: "help-about"
                onTriggered: {
                    openDialog("qrc:/qml/AboutPage.qml")
                }
            }
        ]
    }

    pageStack {
        globalToolBar.showNavigationButtons: isFirstPage ? 0 : Kirigami.ApplicationHeaderStyle.ShowBackButton
        defaultColumnWidth: (itemListProportion * mainWindow.width) - (globalDrawer.actualWidth / 2)
        interactive: false
    }

    function pushAll() {
        pageStack.clear()
        pageStack.push("qrc:/qml/ArticleListPage.qml", {pageRow: pageStack})
    }

    function pushFeed(feed) {
        pageStack.clear()
        pageStack.push("qrc:/qml/ArticleListPage.qml",
                       {pageRow: pageStack,
                           feedFilter: feed.feedId,
                           title: feed.feedName})
    }

    function openDialog(pageUrl, pageProps) {
        var dialogComponent = Qt.createComponent("qrc:/qml/PageDisplayDialog.qml");
        if (dialogComponent.status === Component.Error) {
            console.log("failed to load page for dialog: "+dialogComponent.errorString());
        } else {
            var size = Math.min(mainWindow.width, mainWindow.height) * 0.62
            var properties = {
                pageSource: pageUrl,
                pageProps: pageProps || {},
                height: size,
                width: size
            }

            var dialog = dialogComponent.createObject(mainWindow, properties);
            dialog.show()
        }
    }

    property list<State> states: [
        State {
            name: "portrait"
            when: width < height
            PropertyChanges {
                target: mainWindow
                itemListProportion: 1
                feedListProportion: 0.23
            }
            PropertyChanges {
                target: globalDrawer
                drawerOpen: false
                modal: true
            }
        },

        State {
            name: "widescreen"
            when: width > (height *1.6)
            PropertyChanges {
                target: mainWindow
                itemListProportion: 0.38
                feedListProportion: 0.15
            }
            PropertyChanges {
                /* separated to avoid stacking errors */
                target: globalDrawer
                drawerOpen: true
            }
            PropertyChanges {
                target: globalDrawer
                modal: false
            }
        }
    ]

    StateGroup {
        states: mainWindow.states
    }
}
