/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick 2.12
import QtQuick.Layouts 1.12
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
    supportsRefreshing: true
    refreshing: isUpdating
    signal suspendAnimations

    topPadding: 0
    bottomPadding: 0
    leftPadding: 0
    rightPadding: 0
    title: feed ? feed.name : ""
    Kirigami.Theme.colorSet: Kirigami.Theme.View
    Kirigami.Theme.backgroundColor: Kirigami.Theme.alternateBackgroundColor

    EmptyFeedOverlay {
        id: emptyOverlay
        anchors.fill: parent
        visible: articleList.count == 0
        icon: unreadFilter ? "checkmark" : "com.rocksandpaper.syndic.feed-empty"
        text: unreadFilter ? qsTr("All Read") : qsTr("No Items")
    }

    ListView {
        id: articleList
        clip: true
        anchors.fill: parent
        currentIndex: -1

        model: ArticleListModel {
           id: feedItemModel
           feed: root.feed
           unreadFilter: root.unreadFilter
           onStatusChanged: {
               if (articleList.currentItem || !automaticOpen) return;
               if ((model.status === Feed.Idle) && (model.rowCount() > 0))
                   articleList.currentIndex = 0;
               else openChild();
           }
       } /* model */

        delegate: Kirigami.AbstractListItem {
            width: articleList.width
            text: ref.article.headline
            padding: 10

            // if we don't override this then AbstractListItem sets the height to 0 when the ListView is hidden,
            // which causes ListView to instantiate a very large number of delegates.
            height: implicitHeight

            contentItem: ArticleListEntry { }
            property var data: ref
            onClicked: {
                if (articleList.currentIndex !== model.index) {
                    articleList.currentIndex = model.index
                } else {
                    articleList.currentItemChanged()
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

        onCurrentItemChanged: openChild();

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

    onAutomaticOpenChanged: {
        if (automaticOpen &&  pageRow.lastItem === root) {
            openChild()
        }
    }

    function clearRead() {
        root.currentIndex = model.status === Feed.Idle ? 0 : -1
        root.model.removeRead()
        articleList.positionViewAtBeginning()
    }

    function pushPlaceholder() {
        switch(model.status) {
        case Feed.Loading:
            // just wait for the load...
            break;
        case Feed.Updating:
            root.pageRow.push("qrc:/qml/Placeholders/UpdatingPlaceholderPage.qml");
            break;
        case Feed.Error:
            root.pageRow.push("qrc:/qml/Placeholders/ErrorPlaceholderPage.qml", {model:model});
            break;
        default:
            root.pageRow.push("qrc:/qml/Placeholders/EmptyFeedPlaceholderPage.qml");
        }
    }

    function openChild() {
        if (!pageRow) return;
        pageRow.currentIndex = Kirigami.ColumnView.index
        if (pageRow.wideMode) {
            suspendAnimations();
        }
        if (articleList.currentItem) {
            const data = articleList.currentItem.data
            root.pageRow.push("qrc:/qml/ArticlePage.qml", {item: data, nextItem: nextItem, previousItem: previousItem})
            data.article.isRead = true
        } else if (model && automaticOpen) {
            pushPlaceholder();
        } else {
            root.pageRow.pop(root)
        }
    }

    function nextItem () {
        articleList.currentIndex++;
        articleList.pageDownIfNecessary();
    }
    function previousItem () { articleList.currentIndex-- }
}
