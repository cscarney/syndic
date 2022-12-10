/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import org.kde.kirigami 2.14 as Kirigami
import com.rocksandpaper.syndic 1.0

Kirigami.Page {
    id: root
    property alias item: articleView.item
    property alias isReadable: readableAction.checked
    property var nextItem: function() {}
    property var previousItem: function() {}
    signal suspendAnimations;
    property bool inProgress: false;
    property bool defaultReadable: item.article ? item.article.feed.flags & Feed.UseReadableContentFlag : false

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
            }

        ]
    }

    Connections {
        target: item.article
        function onGotContent(content, type) {
            if (root.isReadable || (type===Article.FeedContent)) {
                root.inProgress = false;
                articleView.text = content;
            }
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
        requestContent();
        root.isReadableChanged.connect(requestContent)
    }

    function requestContent() {
        root.inProgress = true;
        if (root.isReadable) {
            item.article.requestReadableContent(feedContext);
        } else {
            item.article.requestContent();
        }
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
