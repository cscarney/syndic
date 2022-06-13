/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Window 2.15
import org.kde.kirigami 2.7 as Kirigami

Kirigami.ApplicationWindow {
    id: root
    title: (priv.pageTitle.length>0 ? priv.pageTitle+" - " : "") + Qt.application.displayName
    width: globalSettings.width
    height: globalSettings.height
    font: Kirigami.Theme.defaultFont

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
            leftMargin: (pageStack.firstVisibleItem ? pageStack.firstVisibleItem.width : 0);
        }
        width: 10
        cursorShape: Qt.SizeHorCursor;
        enabled: pageStack.firstVisibleItem && !Kirigami.Settings.hasTransientTouchInput
        onPositionChanged: {
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

        topContent: ColumnLayout {
            spacing: 0
            Layout.fillWidth: true

            FeedList {
                id: feedList
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.preferredHeight: drawer.height
                onCurrentlySelectedFeedChanged:
                    if (currentlySelectedFeed) pushFeed(currentlySelectedFeed)
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
                text: qsTr("Add Content")
                iconName: "list-add"
                onTriggered: {
                    pushUtilityPage("qrc:/qml/AddFeedPage.qml", {pageRow: pageStack})
                }
            },
            Kirigami.Action {
                text: qsTr("Settings")
                iconName: "settings-configure"
                onTriggered: {
                    pushUtilityPage("qrc:/qml/SettingsPage.qml")
                }
            },
            Kirigami.Action {
                text: qsTr("About %1").arg(Qt.application.displayName)
                iconName: "help-about"
                onTriggered: {
                    pushUtilityPage("qrc:/qml/AboutPage.qml")
                }
            }
        ]
    }

    StateGroup {
        states: [
            State {
                name: "portrait"
                when: width < height
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
                when: (!Kirigami.Settings.isMobile) && (width > (height *1.6))
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
        property real itemListProportion:  0.38
        property real feedListProportion: 0.23
        property bool isFirstPage: pageStack.firstVisibleItem === pageStack.items[0]
        property string pageTitle: pageStack.firstVisibleItem ? pageStack.firstVisibleItem.title : ""
    }

    Timer {
        id: animationSuspendTimer
        interval: Kirigami.Units.longDuration
    }

    Connections {
        target: pageStack.currentItem
        ignoreUnknownSignals: true

        function onBackRequested(event) {
            root.goBack(event);
        }

        function onSuspendAnimations() {
            // emitted by pages to temporarily suspend the page transition animation
            animationSuspendTimer.start()
        }
    }

    Connections {
        target: feedContext
        function onFeedListPopulated(nFeeds) {
            if (nFeeds === 0) {
                pushUtilityPage("qrc:/qml/AddFeedPage.qml", {pageRow: pageStack})
            }
        }
    }

    onClosing: {
        if (root.visibility === Window.Windowed) {
            globalSettings.width = width
            globalSettings.height = height
        }
    }

    Component.onCompleted: {
        // HACK workaround Kirigami's hopelessly broken back button behavior
        // Kirigami doesn't accept the underlying key event when the backRequested
        // event is accepted, so if we don't handle the back button directly we
        // get the fallback behavior of *quitting the application* every time
        // the user hits the back button.
        //
        // To avoid this, we call goBack again with the key event.
        pageStack.Keys.released.connect(function(event) {
            if (event.key === Qt.Key_Back) {
                goBack(event);
            }
        });
    }

    function goBack(event) {
        // go back to the first non-visible page (rather than the
        // default back behavior, which just moves the focus
        // one column to the left)
        const index = pageStack.firstVisibleItem.Kirigami.ColumnView.index;
        if (index > 0) {
            pageStack.currentIndex = index-1;
            event.accepted = true;
        } else if (feedList.currentIndex != 0) {
            feedList.currentIndex = 0;
            event.accepted = true
        }
    }

    function pushFeed(feed) {
        pageStack.clear()
        pageStack.push("qrc:/qml/ArticleList/ArticleListPage.qml",
                       {pageRow: pageStack,
                           feed: feed})
    }

    function pushUtilityPage(pageUrl, pageProps) {
        feedList.currentIndex = -1
        feedList.currentlySelectedFeed = null
        pageStack.clear()
        pageStack.push(pageUrl, pageProps)
    }
}
