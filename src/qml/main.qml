import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import Qt.labs.settings 1.1
import org.kde.kirigami 2.7 as Kirigami

Kirigami.ApplicationWindow {
    id: root
    title: (priv.pageTitle.length>0 ? priv.pageTitle+" - " : "") + qsTr("FeedKeeper")

    pageStack {
        globalToolBar.showNavigationButtons: priv.isFirstPage ? 0 : Kirigami.ApplicationHeaderStyle.ShowBackButton
        defaultColumnWidth: (priv.itemListProportion * root.width) - (globalDrawer.actualWidth / 2)
        interactive: false
    }

    globalDrawer: Kirigami.GlobalDrawer {
        id: drawer
        property real actualWidth: drawerOpen && !modal ? width : 0

        width: priv.feedListProportion * root.width
        modal: true
        handleVisible: modal && priv.isFirstPage
        Kirigami.Theme.colorSet: Kirigami.Theme.View

        topContent: ColumnLayout {
            spacing: 0
            Layout.fillWidth: true

            FeedList {
                id: feedList
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.preferredHeight: drawer.height
                onFeedSelected: if (currentItem.feedRef.feed) pushFeed(currentItem.feedRef)
                onItemClicked: drawer.drawerOpen = !drawer.modal
            }

            Kirigami.Separator {
                Layout.fillWidth: true
                Layout.leftMargin: -drawer.leftPadding
                Layout.rightMargin: -drawer.rightPadding
            }
        }

        actions: [
            Kirigami.Action {
                text: "Add Content..."
                iconName: "list-add"
                onTriggered: {
                    openDialog("qrc:/qml/AddFeedPage.qml")
                }
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

    Settings {
        // @disable-check M16
        category: "Window"
        property alias width: root.width
        property alias height: root.height
    }

    StateGroup {
        states: [
            State {
                name: "portrait"
                when: width < height
                PropertyChanges {
                    target: priv
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
                    target: priv
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
    }

    QtObject {
        id: priv
        property real itemListProportion:  0.38
        property real feedListProportion: 0.23
        property bool isFirstPage: pageStack.firstVisibleItem === pageStack.items[0]
        property string pageTitle: pageStack.firstVisibleItem ? pageStack.firstVisibleItem.title : ""
    }

    function pushFeed(feedRef) {
        pageStack.clear()
        pageStack.push("qrc:/qml/ArticleList/ArticleListPage.qml",
                       {pageRow: pageStack,
                           feedRef: feedRef})
    }

    function openDialog(pageUrl, pageProps) {
        var dialogComponent = Qt.createComponent("qrc:/qml/PageDisplayDialog.qml");
        if (dialogComponent.status === Component.Error) {
            console.log("failed to load page for dialog: "+dialogComponent.errorString());
        } else {
            var size = Math.min(root.width, root.height) * 0.62
            var properties = {
                pageSource: pageUrl,
                pageProps: pageProps || {},
                height: size,
                width: size
            }
            var dialog = dialogComponent.createObject(root, properties);
            dialog.show()
        }
    }
}
