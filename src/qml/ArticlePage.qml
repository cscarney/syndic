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
    required property var parentList
    readonly property bool inProgress: swipeView.currentItem ? !!swipeView.currentItem.inProgress : false;
    readonly property string hoveredLink: swipeView.currentItem ? swipeView.currentItem.hoveredLink || "" : ""
    readonly property var item: swipeView.currentItem ? swipeView.currentItem.item : null;

    topPadding: 0
    bottomPadding: 0
    leftPadding: 0
    rightPadding: 0
    Kirigami.Theme.inherit: false
    Kirigami.Theme.colorSet: Kirigami.Theme.View
    Layout.fillWidth: true
    title: item ? item.article.title || "" : ""
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
                enabled: !!item
                onTriggered: platformHelper.share(item.article.url);
            },

            Kirigami.Action {
                text: qsTr("Starred")
                checkable: true
                checked: item ? item.article.isStarred : false
                enabled: !!item
                onTriggered: item.article.isStarred = checked
                iconName: checked ? "starred-symbolic-nomask" : "non-starred-symbolic-nomask"
                displayHint: Kirigami.DisplayHint.IconOnly
            },

            Kirigami.Action {
                //: as in, don't mark this article as read
                text: qsTr("Keep Unread")
                iconName: "mail-mark-unread"
                checkable: true
                checked: item ? !item.article.isRead : true
                enabled: item ? true : false
                onTriggered: item.article.isRead = !checked
                displayHint: Kirigami.DisplayHint.AlwaysHide
            },

            Kirigami.Action {
                id: readableAction
                iconName: "view-readermode"
                text: qsTr("Show Web Content");
                checkable: true
                checked: swipeView.currentItem ? swipeView.currentItem.isReadable : false
                enabled: swipeView.currentItem ? true : false
                displayHint: Kirigami.DisplayHint.AlwaysHide
                onTriggered: {
                    if (swipeView.currentItem) {
                        swipeView.currentItem.isReadable = !swipeView.currentItem.isReadable;
                    }
                }
            },

            Kirigami.Action {
                id: refreshReadableAction
                text: qsTr("Reload Web Content")
                iconName: "view-refresh"
                visible: swipeView.currentItem && readableAction.checked
                displayHint: Kirigami.DisplayHint.AlwaysHide
                onTriggered: swipeView.currentItem.refreshReadable()
            }

        ]
    }

    ListView {
        id: swipeView
        clip: true
        anchors.fill: parent
        interactive: Kirigami.Settings.hasTransientTouchInput
        orientation: ListView.Horizontal
        highlightMoveDuration: 0

        model: parentList.model
        currentIndex: parentList.currentIndex
        snapMode: ListView.SnapOneItem
        highlightRangeMode: ListView.StrictlyEnforceRange

        delegate: ArticlePageSwipeViewItem {
            required property var ref
            item: ref
            showExpandedByline: (ref.article.feed !== swipeView.model.feed)
            Keys.forwardTo: [root]
            implicitHeight: swipeView.height || 1
            implicitWidth: swipeView.width || 1
        }

        onCurrentItemChanged: {
            if (swipeView.currentIndex < parentList.currentIndex) {
                previousItem();
            } else if (swipeView.currentIndex > parentList.currentIndex) {
                nextItem();
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


    onItemChanged: {
        if (item) {
            item.article.isRead = true;
        }
    }

    Keys.onPressed: {
        if (!root.item) {
            return;
        }

        switch (event.key) {
        case Qt.Key_Space:
            if (!scroller.atYEnd) {
                swipeView.currentItem.pageUpDown(0.9);
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
            swipeView.currentItem.pxUpDown(Kirigami.Units.gridUnit * -2);
            break;

        case Qt.Key_Down:
            swipeView.currentItem.pxUpDown(Kirigami.Units.gridUnit * 2);
            break;

        case Qt.Key_PageUp:
            swipeView.currentItem.pageUpDown(-0.9);
            break;

        case Qt.Key_PageDown:
            swipeView.currentItem.pageUpDown(0.9);
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

    function nextItem () {
        parentList.currentIndex++;
        parentList.pageDownIfNecessary();
    }
    function previousItem () { parentList.currentIndex-- }
}
