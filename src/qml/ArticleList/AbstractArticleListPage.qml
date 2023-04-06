/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Window 2.15
import org.kde.kirigami 2.7 as Kirigami
import com.rocksandpaper.syndic 1.0

Kirigami.ScrollablePage {
    id: root

    property var feed
    property var pageRow: null
    property alias model: articleList.model
    property alias count: articleList.count
    property alias currentIndex: articleList.currentIndex
    property bool isUpdating: model.status === Feed.Updating
    property bool unreadFilter: true
    property bool automaticOpen: pageRow && (pageRow.defaultColumnWidth * 2 < pageRow.width)
    property string childPage: {
        if (articleList.currentItem) {
            return "qrc:/qml/ArticlePage.qml"
        } else if (model && automaticOpen) {
            switch(model.status) {
            case Feed.Loading:
                return ""
            case Feed.Updating:
                return "qrc:/qml/Placeholders/UpdatingPlaceholderPage.qml";
            case Feed.Error:
                return "qrc:/qml/Placeholders/ErrorPlaceholderPage.qml";
            default:
                return "qrc:/qml/Placeholders/EmptyFeedPlaceholderPage.qml";
            }
        } else {
            return "";
        }
    }

    supportsRefreshing: true
    refreshing: isUpdating

    topPadding: 0
    bottomPadding: 0
    leftPadding: 0
    rightPadding: 0
    title: feed ? feed.name : ""
    Kirigami.Theme.colorSet: Kirigami.Theme.View
    Kirigami.Theme.backgroundColor: Kirigami.Theme.alternateBackgroundColor

    Timer {
        id: pageOpenTimer
        interval: 1
        onTriggered: {
            if (!pageRow) {
                return;
            }
            if (pageRow.wideMode) {
                root.Window.window.suspendAnimations();
            }
            pageRow.currentIndex = root.Kirigami.ColumnView.index
            if (childPage) {
                root.pageRow.push(childPage, {model: feedItemModel, parentList: articleList, nextItem: nextItem, previousItem: previousItem})
            } else {
                root.pageRow.pop(root);
            }
        }
    }

    ListView {
        id: articleList
        clip: true
        anchors.fill: parent
        currentIndex: -1
        pressDelay: Kirigami.Settings.hasTransientTouchInput ? 120 : 0

        model: ArticleListModel {
           id: feedItemModel
           feed: root.feed
           unreadFilter: root.unreadFilter
           onStatusChanged: {
               if (articleList.currentItem || !automaticOpen) return;
               if ((model.status === Feed.Idle) && (model.rowCount() > 0))
                   articleList.currentIndex = 0;
           }
           onRowsAboutToBeRemoved: function(parent, first, last){
               // force currentIndex to be updated before we check to see if it was removed
               articleList.forceLayout();
               if (root.currentIndex < first || root.currentIndex > last)
                   return;
               root.currentIndex = -1
           }
       } /* model */

        delegate: Kirigami.AbstractListItem {
            width: articleList.width
            text: ref.article.headline
            padding: 10
            separatorVisible: true

            // if we don't override this then AbstractListItem sets the height to 0 when the ListView is hidden,
            // which causes ListView to instantiate a very large number of delegates.
            height: implicitHeight

            contentItem: ArticleListEntry { }
            property var data: ref
            onClicked: {
                if (articleList.currentIndex !== model.index) {
                    articleList.currentIndex = model.index
                } else {
                    pageRow.currentIndex = root.Kirigami.ColumnView.index + 1
                }
            }
        } /*  delegate */

        header: Kirigami.InlineMessage {
            id: errorMessage
            text: qsTr("Couldn't fetch feed from source")
            width: parent.width
            visible: model.status === Feed.Error
            type: Kirigami.MessageType.Error
            actions: [
                Kirigami.Action {
                    text: qsTr("Retry")
                    onTriggered: {
                        root.model.requestUpdate();
                    }
                }
            ]
        }

        EmptyFeedOverlay {
            id: emptyOverlay
            anchors.fill: parent
            visible: (articleList.count == 0) && (feedItemModel.status !== Feed.Loading)
            icon: unreadFilter ? "checkmark" : "com.rocksandpaper.syndic.feed-empty"
            text: unreadFilter ? qsTr("All Read") : qsTr("No Items")
        }

        function pageDownIfNecessary() {
            if (!currentItem) return;
            const pos = currentItem.y + currentItem.height - contentY;
            if ( pos >= height) {
                positionViewAtIndex(currentIndex, ListView.Beginning);
            }
        }
    } /* articleList */

    onRefreshingChanged: {
        if (refreshing && (model.status === Feed.Idle)) {
            model.requestUpdate();
            refreshing = Qt.binding(()=>isUpdating);
        }
    }

    onChildPageChanged: pageOpenTimer.start()

    function clearRead() {
        root.currentIndex = -1
        root.model.removeRead()
        articleList.positionViewAtBeginning()
    }

    function nextItem () {
        articleList.currentIndex++;
        articleList.pageDownIfNecessary();
    }
    function previousItem () { articleList.currentIndex-- }
}
