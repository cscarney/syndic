/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import org.kde.kirigami 2.14 as Kirigami

Kirigami.Page {
    id: root
    property alias item: articleView.item
    property var nextItem: function() {}
    property var previousItem: function() {}
    signal suspendAnimations;

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
                iconName: "emblem-shared-symbolic"
                displayHint: Kirigami.DisplayHint.AlwaysHide
                onTriggered: platformHelper.share(item.article.url);
            },

            Kirigami.Action {
                text: qsTr("Starred")
                checkable: true
                checked: item.article.isStarred
                onCheckedChanged: item.article.isStarred = checked
                iconName: checked ? "starred-symbolic" : "non-starred-symbolic"
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
            }
        ]
    }

    Connections {
        target: item.article
        function onGotContent(content) {
            articleView.text = content;
        }
    }

    /* HACK this SwipeView contains sentinel items that will result in the current
      page being unloaded.  It would be better to bind to the ArticleListModel
      to a single instance of ArticlePage and sync the SwipeView with the article
      list. */
    SwipeView {
        id: swipeView
        clip: true
        anchors.fill: parent
        currentIndex: 1
        interactive: Kirigami.Settings.hasTransientTouchInput

        Item {
            id: previousItemSentinel
            Connections {
                target: swipeView.contentItem
                function onMovementEnded() {
                    if (previousItemSentinel.SwipeView.isCurrentItem) {
                        suspendAnimations();
                        previousItem();
                    }
                }
            }
        }


        ScrollView {
            id: scrollView
            clip: true
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

            Flickable {
                id: scroller
                anchors.fill: parent
                Keys.forwardTo: [root]

                contentWidth: root.width - leftMargin - rightMargin
                contentHeight: articleView.height + bottomMargin + topMargin
                clip: true
                flickableDirection: Flickable.AutoFlickIfNeeded
                topMargin: Kirigami.Units.gridUnit
                bottomMargin: Kirigami.Units.gridUnit
                leftMargin: Kirigami.Units.gridUnit * 1.6
                rightMargin: Kirigami.Units.gridUnit * 1.6
                ArticleView {
                    id: articleView
                    width: scroller.contentWidth
                }

                Behavior on contentY {
                    id: animateScroll
                    NumberAnimation {
                        duration: Kirigami.Units.shortDuration
                        easing.type: Easing.OutExpo
                    }
                }
            }
        }

        Item {
            id: nextItemSentinel
            Connections {
                target: swipeView.contentItem
                function onMovementEnded() {
                    if (nextItemSentinel.SwipeView.isCurrentItem) {
                        suspendAnimations();
                        nextItem();
                    }
                }
            }
        }
    }

    OverlayMessage {
        id: hoveredLinkToolTip
        text: articleView.hoveredLink
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
    }

    Keys.onPressed: {
        switch (event.key) {
        case Qt.Key_Space:
            if (!scroller.atYEnd) {
                pageUpDown(0.9);
            } else {
                suspendAnimations();
                nextItem();
            }
            break;

        case Qt.Key_Left:
            suspendAnimations();
            previousItem();
            break;

        case Qt.Key_Right:
            suspendAnimations();
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

    Component.onCompleted: {
        item.article.requestContent();
    }

    function pxUpDown(increment) {
        const topY = scroller.originY - scroller.topMargin;
        const bottomY = scroller.originY + scroller.contentHeight + scroller.bottomMargin - scroller.height;
        scroller.contentY = Math.max(topY, Math.min(scroller.contentY + increment, bottomY))
    }

    function pageUpDown(increment) {
        pxUpDown(increment * scroller.height)
    }
}
