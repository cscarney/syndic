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
    required property var articleListController
    readonly property bool inProgress: !!swipeView.currentItem?.inProgress
    readonly property string hoveredLink: swipeView.currentItem?.hoveredLink ?? ""
    readonly property Article currentArticle: swipeView.currentItem ? swipeView.currentItem.article : null;

    topPadding: 0
    bottomPadding: 0
    leftPadding: 0
    rightPadding: 0
    Kirigami.Theme.inherit: false
    Kirigami.Theme.colorSet: Kirigami.Theme.View
    Layout.fillWidth: true
    title: currentArticle?.title ?? ""
    titleDelegate: Item { }

    actions: [
        Kirigami.Action {
            text: qsTr("Open")
            icon.name: "globe"
            onTriggered: Qt.openUrlExternally(currentArticle.url);
            displayHint: Kirigami.Settings.isMobile ? Kirigami.DisplayHint.IconOnly : Kirigami.DisplayHint.NoPreference
        },

        Kirigami.Action {
            text: qsTr("Share")
            icon.name: "emblem-shared-symbolic-nomask"
            displayHint: Kirigami.DisplayHint.AlwaysHide
            enabled: !!currentArticle
            onTriggered: platformHelper.share(currentArticle.url);
        },

        Kirigami.Action {
            text: qsTr("Starred")
            checkable: true
            checked: !!currentArticle?.isStarred
            enabled: !!currentArticle
            onTriggered: currentArticle.isStarred = checked
            icon.name: checked ? "starred-symbolic-nomask" : "non-starred-symbolic-nomask"
            displayHint: Kirigami.DisplayHint.IconOnly
        },

        Kirigami.Action {
            //: as in, don't mark this article as read
            text: qsTr("Keep Unread")
            icon.name: "mail-mark-unread"
            checkable: true
            checked: currentArticle ? !currentArticle.isRead : true
            enabled: !!currentArticle
            onTriggered: currentArticle.isRead = !checked
            displayHint: Kirigami.DisplayHint.AlwaysHide
        },

        Kirigami.Action {
            id: readableAction
            icon.name: "view-readermode"
            text: qsTr("Show Web Content");
            checkable: true
            checked: !!swipeView.currentItem?.isReadable
            enabled: !!swipeView.currentItem
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
            icon.name: "view-refresh"
            visible: swipeView.currentItem && readableAction.checked
            displayHint: Kirigami.DisplayHint.AlwaysHide
            onTriggered: swipeView.currentItem.refreshReadable()
        }
    ]

    ListView {
        id: swipeView
        clip: true
        anchors.fill: parent
        interactive: Kirigami.Settings.hasTransientTouchInput
        orientation: ListView.Horizontal
        highlightMoveDuration: 0

        model: articleListController.model
        currentIndex: articleListController.currentIndex
        snapMode: ListView.SnapOneItem
        highlightRangeMode: ListView.StrictlyEnforceRange

        delegate: ArticlePageSwipeViewItem {
            required property var ref
            article: ref.article
            showExpandedByline: (ref.article.feed !== swipeView.model.feed)
            Keys.forwardTo: [root]
            implicitHeight: swipeView.height || 1
            implicitWidth: swipeView.width || 1
        }

        onCurrentItemChanged: {
            if (swipeView.currentIndex < articleListController.currentIndex) {
                articleListController.previousItem();
            } else if (swipeView.currentIndex > articleListController.currentIndex) {
                articleListController.nextItem();
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


    onCurrentArticleChanged: {
        if (currentArticle) {
            currentArticle.isRead = true;
        }
    }

    component ArticleNavigationShortcut: Shortcut {
        property bool ignoreWhen: false
        enabled: (root.currentArticle != null) && (!ignoreWhen)
    }


    property list<Shortcut> shortcuts: [
        ArticleNavigationShortcut {
            sequences: [" "]
            onActivated: {
                if (!swipeView.currentItem.atYEnd) {
                    swipeView.currentItem.pageUpDown(0.9);
                } else {
                    articleListController.nextItem();
                }
            }
        },

        ArticleNavigationShortcut {
            sequences: ["Left"]
            onActivated: {
                articleListController.previousItem();
            }
        },

        ArticleNavigationShortcut {
            sequences: ["Right"]
            onActivated: {
                articleListController.nextItem();
            }
        },

        ArticleNavigationShortcut {
            sequences: ["Up"]
            onActivated: {
                swipeView.currentItem.pxUpDown(Kirigami.Units.gridUnit * -2);
            }
        },

        ArticleNavigationShortcut {
            sequences: ["Down"]
            onActivated: {
                swipeView.currentItem.pxUpDown(Kirigami.Units.gridUnit * 2);
            }
        },

        ArticleNavigationShortcut {
            sequences: ["PgUp"]
            onActivated: {
                swipeView.currentItem.pageUpDown(-0.9);
            }
        },

        ArticleNavigationShortcut {
            sequences: ["PgDown"]
            onActivated: {
                swipeView.currentItem.pageUpDown(0.9);
            }
        },

        ArticleNavigationShortcut {
            sequences: ["Home"]
            onActivated: {
                swipeView.currentItem.scrollToTop();
            }
        },

        ArticleNavigationShortcut {
            sequences: ["End"]
            onActivated: {
                swipeView.currentItem.scrollToBottom();
            }
        },

        ArticleNavigationShortcut {
            sequences: ["Return", "Enter"]
            ignoreWhen: root.ApplicationWindow.activeFocusControl instanceof TextField
            onActivated: {
                Qt.openUrlExternally(currentArticle.url);
            }
        }
    ]
}
