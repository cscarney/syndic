/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Window 2.15
import org.kde.kirigami 2.14 as Kirigami
import com.rocksandpaper.syndic 1.0

Kirigami.Page {
    id: root
    property var item
    property alias isReadable: readableAction.checked
    property bool showExpandedByline: false
    property var nextItem: function() {}
    property var previousItem: function() {}
    property bool inProgress: false;
    property bool defaultReadable: item.article ? item.article.feed.flags & Feed.UseReadableContentFlag : false
    property string hoveredLink: swipeView.currentItem ? swipeView.currentItem.hoveredLink || "" : ""

    topPadding: 0
    bottomPadding: 0
    leftPadding: 0
    rightPadding: 0
    Kirigami.Theme.inherit: false
    Kirigami.Theme.colorSet: Kirigami.Theme.View
    Layout.fillWidth: true
    title:  item.article.title || ""
    titleDelegate: Item { }

    actions {
        main: Kirigami.Action {
            text: qsTr("Open")
            iconName: "globe"
            onTriggered: Qt.openUrlExternally(item.article.url);
            displayHint: Kirigami.Settings.isMobile ? Kirigami.DisplayHint.IconOnly : Kirigami.DisplayHint.NoPreference
        }

        contextualActions: [
            Kirigami.Action {
                text: qsTr("Share")
                iconName: "emblem-shared-symbolic-nomask"
                displayHint: Kirigami.DisplayHint.AlwaysHide
                onTriggered: platformHelper.share(item.article.url);
            },

            Kirigami.Action {
                text: qsTr("Starred")
                checkable: true
                checked: item.article.isStarred
                onCheckedChanged: item.article.isStarred = checked
                iconName: checked ? "starred-symbolic-nomask" : "non-starred-symbolic-nomask"
                displayHint: Kirigami.DisplayHint.IconOnly
            },

            Kirigami.Action {
                //: as in, don't mark this article as read
                text: qsTr("Keep Unread")
                iconName: "mail-mark-unread"
                checkable: true
                checked: false
                onCheckedChanged: item.article.isRead = !checked
                displayHint: Kirigami.DisplayHint.AlwaysHide
            },

            Kirigami.Action {
                id: readableAction
                iconName: "view-readermode"
                text: qsTr("Show Web Content");
                checkable: true
                checked: defaultReadable
                displayHint: Kirigami.DisplayHint.AlwaysHide
            },

            Kirigami.Action {
                id: refreshReadableAction
                text: qsTr("Reload Web Content")
                iconName: "view-refresh"
                visible: isReadable
                displayHint: Kirigami.DisplayHint.AlwaysHide
                onTriggered: refreshReadable()
            }

        ]
    }

    /* HACK this SwipeView contains sentinel items that will result in the current
      page being unloaded.  It would be better to bind to the ArticleListModel
      to a single instance of ArticlePage and sync the SwipeView with the article
      list. */
    SwipeView {
        id: swipeView
        clip: true
        anchors.fill: parent
        interactive: Kirigami.Settings.hasTransientTouchInput

        Item {
            id: previousItemSentinel
            Connections {
                target: swipeView.contentItem
                function onMovementEnded() {
                    if (previousItemSentinel.SwipeView.isCurrentItem) {
                        root.Window.window.suspendAnimations();
                        previousItem();
                    }
                }
            }
        }


        Repeater {
            model: 1
            delegate: ArticlePageSwipeViewItem {
                item: root.item
                isReadable: root.isReadable
                showExpandedByline: root.showExpandedByline
                Component.onCompleted: swipeView.currentIndex = this.SwipeView.index
            }
        }

        Item {
            id: nextItemSentinel
            Connections {
                target: swipeView.contentItem
                function onMovementEnded() {
                    if (nextItemSentinel.SwipeView.isCurrentItem) {
                        root.Window.window.suspendAnimations();
                        nextItem();
                    }
                }
            }
        }
    }

    OverlayMessage {
        id: hoveredLinkToolTip
        text: root.hoveredLink
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
    }

    BusyIndicator {
        id: loadingContentIndicator
        anchors.centerIn: parent
        visible: root.inProgress && !spinnerDelayTimer.running
    }

    Timer {
        id: spinnerDelayTimer;
        interval: Kirigami.Units.humanMoment;
        running: root.inProgress
    }


    Keys.onPressed: {
        switch (event.key) {
        case Qt.Key_Space:
            if (!scroller.atYEnd) {
                pageUpDown(0.9);
            } else {
                root.Window.window.suspendAnimations();
                nextItem();
            }
            break;

        case Qt.Key_Left:
            root.Window.window.suspendAnimations();
            previousItem();
            break;

        case Qt.Key_Right:
            root.Window.window.suspendAnimations();
            nextItem();
            break;

        case Qt.Key_Up:
            pxUpDown(Kirigami.Units.gridUnit * -2);
            break;

        case Qt.Key_Down:
            pxUpDown(Kirigami.Units.gridUnit * 2);
            break;

        case Qt.Key_PageUp:
            pageUpDown(-0.9);
            break;

        case Qt.Key_PageDown:
            pageUpDown(0.9);
            break;

        case Qt.Key_Return:
        case Qt.Key_Enter:
            Qt.openUrlExternally(item.article.url);
            break;

        default:
            return;
        }
        event.accepted = true
    }
}
