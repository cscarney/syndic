/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQml 2.15
import com.rocksandpaper.syndic 1.0
import org.kde.kirigami 2.14 as Kirigami

AbstractArticleListPage {
    id: root
    unreadFilter: globalSettings.unreadFilter
    property bool acceptFeedListAction: false

    actions: [
        Kirigami.Action {
            text: qsTr("Mark All Read")
            iconName: "checkmark"
            enabled: root.feed && root.feed.unreadCount > 0
            onTriggered: {
                root.model.markAllRead();
            }
            displayHint: Kirigami.DisplayHint.KeepVisible
        },

        Kirigami.Action {
             text: qsTr("Hide Read")
             iconName: "view-filter"
             checkable: true
             checked: globalSettings.unreadFilter
             displayHint: Kirigami.DisplayHint.AlwaysHide
             onCheckedChanged: globalSettings.unreadFilter = checked
         },

        Kirigami.Action {
            text: qsTr("Edit…")
            iconName: "document-edit"
            displayHint: Kirigami.DisplayHint.AlwaysHide
            visible: feed && feed.editable
            onTriggered: {
                pageRow.pop(root)
                pageRow.push("qrc:/qml/EditFeedPage.qml", {targetFeed: feed, pageRow: root.pageRow, onDone:root.childPageChanged});
            }
        },

        Kirigami.Action {
            text: qsTr("Refresh")
            iconName: "view-refresh"
            displayHint: Kirigami.DisplayHint.AlwaysHide
            enabled: feed && feed.status!=Feed.Updating
            visible: !cancelAction.visible
            onTriggered: {
                root.model.requestUpdate();
            }
        },

        Kirigami.Action {
            id: cancelAction
            text: qsTr("Cancel")
            iconName: "dialog-cancel"
            displayHint: Kirigami.DisplayHint.AlwaysHide
            visible: feed && feed.status===Feed.Updating
            onTriggered: {
                feed.updater.abort()
            }
        }
    ]

    function feedListAction() {
        // ignore the one that created us...
        if (!acceptFeedListAction) return;
        clearRead();
    }

    Component.onCompleted: Qt.callLater(function(){
        acceptFeedListAction = true;
    })
}
