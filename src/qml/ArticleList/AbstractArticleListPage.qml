/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick 2.12
import QtQuick.Controls
import QtQuick.Layouts 1.12
import QtQuick.Window 2.15
import org.kde.kirigami 2.7 as Kirigami
import com.rocksandpaper.syndic

Kirigami.ScrollablePage {
    id: root
    required property Item pageRow
    required property ArticleListModel model
    property alias delegate: articleList.delegate
    property alias count: articleList.count
    property alias currentIndex: articleList.currentIndex
    readonly property ListView view: articleList
    property bool isUpdating: model.status === Feed.Updating
    property bool unreadFilter: true
    property bool automaticOpen: pageRow.defaultColumnWidth * 2 < pageRow.width
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
    title: feed?.name ?? ""
    Kirigami.Theme.colorSet: Kirigami.Theme.View
    Kirigami.Theme.backgroundColor: Kirigami.Theme.alternateBackgroundColor

    // Interface exposed to child pages via the articleListController property
    QtObject {
        id: articleListController
        readonly property ArticleListModel model: root.model
        readonly property int currentIndex: articleList.currentIndex

        function nextItem () {
            articleList.currentIndex++;
            articleList.pageDownIfNecessary();
        }

        function previousItem () { articleList.currentIndex-- }
    }

    Timer {
        id: pageOpenTimer
        interval: 0

        // HACK temporarily disable the scroll animation on the page row, otherwise
        // the article page won't get scrolled into view on the add feed/preview page
        // TODO This seems like a bug in pageRow's animation logic, can we fix it upstream?
        onTriggered: callWithAnimationDisabled(()=>{
            pageRow.currentIndex = root.Kirigami.ColumnView.index
            if (childPage) {
                root.pageRow.push(childPage, {articleListController: articleListController})
            } else {
                root.pageRow.pop(root);
            }
        })

        function callWithAnimationDisabled(f) {
            var pageRowDuration = root.pageRow.columnView.scrollDuration;
            root.pageRow.columnView.scrollDuration = 0;
            f();
            root.pageRow.columnView.scrollDuration = pageRowDuration;
        }
    }

    Connections {
        target: root.model
        function onStatusChanged() {
            if (articleList.currentItem || !automaticOpen) return;
            if ((model.status === Feed.Idle) && (model.rowCount() > 0))
                articleList.currentIndex = 0;
        }

        function onRowsAboutToBeRemoved(parent, first, last) {
            // force currentIndex to be updated before we check to see if it was removed
            articleList.forceLayout();
            if (root.currentIndex < first || root.currentIndex > last)
                return;
            root.currentIndex = -1
        }
    }

    ListView {
        id: articleList
        clip: true
        anchors.fill: parent
        currentIndex: -1
        pressDelay: Kirigami.Settings.hasTransientTouchInput ? 120 : 0

        model: root.model

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
            visible: (articleList.count == 0) && (root.model.status !== Feed.Loading)
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

    function selectIndex(index) {
        articleList.currentIndex = index;

        // close the feed editor if necessary
        // TODO this should really emit a signal on articleListController instead of poking at  editor internals,
        // but articleListController doesn't exist until the Qt6 migration stuff lands.
        if (pageRow.lastItem.onDone) {
            pageRow.lastItem.onDone();
        }
        pageRow.currentIndex = root.Kirigami.ColumnView.index + 1
    }
}
