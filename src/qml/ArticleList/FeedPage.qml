/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQml 2.15
import com.rocksandpaper.syndic 1.0
import org.kde.kirigami 2.14 as Kirigami
import ".."

AbstractFeedPage {
    id: root
    unreadFilter: globalSettings.unreadFilter

    actions: [
        PageAction {
            actionId: "feed.markAllRead"
            text: qsTr("Mark All Read")
            icon.name: "checkmark"
            enabled: root.feed && root.feed.unreadCount > 0
            onTriggered: {
                root.model.markAllRead();
            }
            displayHint: Kirigami.DisplayHint.KeepVisible
        },

        PageAction {
             actionId: "feed.hideRead"
             text: qsTr("Hide Read")
             icon.name: "view-filter"
             checkable: true
             checked: globalSettings.unreadFilter
             displayHint: Kirigami.DisplayHint.AlwaysHide
             onCheckedChanged: (checked)=>{globalSettings.unreadFilter = checked}
         },

        PageAction {
            actionId: "feed.edit"
            text: qsTr("Edit…")
            icon.name: "document-edit"
            displayHint: Kirigami.DisplayHint.AlwaysHide
            visible: feed && feed.editable
            onTriggered: {
                pageRow.pop(root)
                pageRow.push("qrc:/qml/EditFeedPage.qml", {targetFeed: feed, onDone:()=>root.childPageChanged()});
            }
        },

        PageAction {
            actionId: "feed.refresh"
            text: qsTr("Refresh")
            icon.name: "view-refresh"
            displayHint: Kirigami.DisplayHint.AlwaysHide
            enabled: feed && feed.status!=Feed.Updating
            visible: !cancelAction.visible
            onTriggered: {
                root.model.requestUpdate();
            }
        },

        PageAction {
            id: cancelAction
            actionId: "feed.cancel"
            text: qsTr("Cancel")
            icon.name: "dialog-cancel"
            displayHint: Kirigami.DisplayHint.AlwaysHide
            enabled: feed && feed.status===Feed.Updating
            visible: enabled
            onTriggered: {
                feed.updater.abort()
            }
        }
    ]
}
