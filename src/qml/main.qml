/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Window 2.15
import org.kde.kirigami as Kirigami

Kirigami.ApplicationWindow {
    id: root
    title: (priv.pageTitle.length>0 ? priv.pageTitle+" - " : "") + Qt.application.displayName
    width: globalSettings.width
    height: globalSettings.height
    visibility: globalSettings.isMaximized ? Window.Maximized : Window.AutomaticVisibility
    font {
        family: Kirigami.Theme.defaultFont.family
        pointSize: Kirigami.Theme.defaultFont.pointSize
    }

    pageStack {
        globalToolBar.style: Kirigami.ApplicationHeaderStyle.ToolBar
        globalToolBar.showNavigationButtons: priv.isFirstPage ? 0 : Kirigami.ApplicationHeaderStyle.ShowBackButton
        defaultColumnWidth: (priv.itemListProportion * root.width) - (globalDrawer.actualWidth / 2)
        interactive: false
        columnView.scrollDuration: Kirigami.Units.longDuration
    }

    MouseArea {
        id: resizeMouse
        property real xAnchor: 0
        anchors {
            top: pageStack.top
            bottom: pageStack.bottom
            left: pageStack.left
            leftMargin: (pageStack.leadingVisibleItem?.width ?? 0);
        }
        width: 10
        cursorShape: Qt.SizeHorCursor;
        enabled: pageStack.leadingVisibleItem && !Kirigami.Settings.hasTransientTouchInput
        onPositionChanged: (mouse)=>{
            const proportion = (resizeMouse.x + mouse.x +globalDrawer.actualWidth / 2) / root.width;
            priv.itemListProportion = Math.max(Math.min(proportion, 0.49), 0.23);
        }
    }

    globalDrawer: Kirigami.GlobalDrawer {
        id: drawer
        property real actualWidth: drawerOpen && !modal ? width : 0

        width: priv.feedListProportion * root.width
        handleVisible: modal && priv.isFirstPage
        Kirigami.Theme.colorSet: Kirigami.Theme.View

        // Drawer is modal unless a the current page requests to keep it open (e.g. settings)
        modal: !(pageStack.depth===1 && pageStack.items[0].keepDrawerOpen)
        onModalChanged: drawerOpen=!modal

        header: Kirigami.AbstractApplicationHeader {
            Kirigami.SearchField {
                anchors {
                    left: parent.left
                    right: parent.right
                    leftMargin: Kirigami.Units.largeSpacing
                    rightMargin: Kirigami.Units.largeSpacing
                }
                autoAccept: false

                onAccepted: {
                    if (text) {
                        priv.pushUtilityPage("qrc:/qml/ArticleList/SearchResultPage.qml", {pageRow: pageStack, query: text})
                    } else {
                        feedList.currentIndex = 0
                    }
                    drawer.drawerOpen = !drawer.modal
                }
            }
        }

        topContent: ColumnLayout {
            spacing: 0
            Layout.fillWidth: true

            FeedList {
                id: feedList
                interactive: false
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.minimumHeight: contentHeight
                onCurrentlySelectedFeedChanged:
                    if (currentlySelectedFeed) priv.pushFeed(currentlySelectedFeed)

                onItemClicked: {
                    if (pageStack.items[0] && pageStack.items[0].feedListAction) {
                        pageStack.items[0].feedListAction();
                    }
                    drawer.drawerOpen = !drawer.modal
                }
            }

            Kirigami.Separator {
                Layout.fillWidth: true
                Layout.leftMargin: -drawer.leftPadding
                Layout.rightMargin: -drawer.rightPadding
            }
        }

        actions: [
            Kirigami.Action {
                id: highlightsAction
                text: qsTr("Highlights")
                icon.name: "go-home-symbolic"
                onTriggered: {
                    priv.pushUtilityPage("qrc:/qml/ArticleList/OverviewPage.qml", {pageRow: pageStack})
                }
            },

            Kirigami.Action {
                text: qsTr("Add Content")
                icon.name: "list-add"
                onTriggered: {
                    priv.pushUtilityPage("qrc:/qml/AddFeedPage.qml", {pageRow: pageStack})
                }
            },
            Kirigami.Action {
                text: qsTr("Settings")
                icon.name: "settings-configure"
                onTriggered: {
                    priv.pushUtilityPage("qrc:/qml/SettingsPage.qml")
                }
            },
            Kirigami.Action {
                text: qsTr("About %1").arg(Qt.application.displayName)
                icon.name: "help-about"
                onTriggered: {
                    priv.pushUtilityPage("qrc:/qml/AboutPage.qml")
                }
            }
        ]
    }

    StateGroup {
        states: [
            State {
                name: "portrait"
                when: priv.columnCount===1
                PropertyChanges {
                    target: priv
                    itemListProportion: 1
                }
                PropertyChanges {
                    target: drawer
                    modal: true
                    width: drawer.implicitWidth
                }
            },

            State {
                name: "widescreen"
                when: priv.columnCount===3
                PropertyChanges {
                    target: priv
                    feedListProportion: 0.15
                }
                PropertyChanges {
                    target: drawer
                    modal: false
                }
            }
        ]
    }

    QtObject {
        id: priv
        property int columnCount: globalSettings.columnCount ? globalSettings.columnCount :
                                    width < height ? 1 :
                                    (!Kirigami.Settings.isMobile) && (width > (height * 1.6)) ? 3 : 2


        property real itemListProportion:  0.38
        property real feedListProportion: 0.23
        property bool isFirstPage: pageStack.leadingVisibleItem === pageStack.items[0]
        property string pageTitle: pageStack.leadingVisibleItem?.title ?? ""

        function goBack(event) {
            // go back to the first non-visible page (rather than the
            // default back behavior, which just moves the focus
            // one column to the left)
            const index = pageStack.leadingVisibleItem.Kirigami.ColumnView.index;
            if (index > 0) {
                pageStack.currentIndex = index-1;
                event.accepted = true;
            } else if (feedList.currentIndex != 0) {
                // call later b/c goBack might be called more than once
                // due to the back button hack above.
                Qt.callLater(()=>{feedList.currentIndex = 0});
                event.accepted = true
            }
        }

        function pushRoot(pageUrl, pageProps) {
            pageStack.currentIndex = 0;
            pageStack[pageStack.depth ? "replace" : "push"](pageUrl, pageProps)
        }

        function pushFeed(feed) {
            let pageUrl = feedList.pageUrlFor(feed);
            priv.pushRoot(pageUrl,
                           {pageRow: pageStack,
                               feed: feed})
        }

        function pushUtilityPage(pageUrl, pageProps) {
            feedList.currentIndex = -1
            feedList.currentlySelectedFeed = null
            priv.pushRoot(pageUrl, pageProps)
        }

    }

    Connections {
        target: pageStack.currentItem
        ignoreUnknownSignals: true

        function onBackRequested(event) {
            priv.goBack(event);
        }
    }

    Connections {
        target: feedContext
        function onFirstRun() {
            // call later to avoid pushing into an incomplete pageRow
            Qt.callLater(()=>priv.pushUtilityPage("qrc:/qml/AddFeedPage.qml", {pageRow: pageStack}))
        }
    }

    onClosing: {
        globalSettings.isMaximized = (root.visibility === Window.Maximized)
        if (root.visibility === Window.Windowed) {
            globalSettings.width = width
            globalSettings.height = height
        }
    }

    Component.onCompleted: {
        switch (globalSettings.startPage) {
        default:
        case 0:
            highlightsAction.trigger();
            break;

        case 1:
            feedList.currentIndex = 0;
            break;
        }


        // HACK workaround Kirigami's hopelessly broken back button behavior
        // Kirigami doesn't accept the underlying key event when the backRequested
        // event is accepted, so if we don't handle the back button directly we
        // get the fallback behavior of *quitting the application* every time
        // the user hits the back button.
        //
        // To avoid this, we call goBack again with the key event.
        pageStack.Keys.released.connect(function(event) {
            if (event.key === Qt.Key_Back) {
                priv.goBack(event);
            }
        });
    }

    function selectFeed(feed) {
        feedList.selectFeed(feed);
    }
}
