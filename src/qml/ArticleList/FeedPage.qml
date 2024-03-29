/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQml 2.15
import com.rocksandpaper.syndic 1.0
import org.kde.kirigami 2.14 as Kirigami

AbstractFeedPage {
    id: root
    unreadFilter: globalSettings.unreadFilter

    actions: [
        Kirigami.Action {
            text: qsTr("Mark All Read")
            icon.name: "checkmark"
            enabled: root.feed && root.feed.unreadCount > 0
            onTriggered: {
                root.model.markAllRead();
            }
            displayHint: Kirigami.DisplayHint.KeepVisible
        },

        Kirigami.Action {
             text: qsTr("Hide Read")
             icon.name: "view-filter"
             checkable: true
             checked: globalSettings.unreadFilter
             displayHint: Kirigami.DisplayHint.AlwaysHide
             onCheckedChanged: (checked)=>{globalSettings.unreadFilter = checked}
         },

        Kirigami.Action {
            text: qsTr("Edit…")
            icon.name: "document-edit"
            displayHint: Kirigami.DisplayHint.AlwaysHide
            visible: feed && feed.editable
            onTriggered: {
                pageRow.pop(root)
                pageRow.push("qrc:/qml/EditFeedPage.qml", {targetFeed: feed, onDone:()=>root.childPageChanged()});
            }
        },

        Kirigami.Action {
            text: qsTr("Refresh")
            icon.name: "view-refresh"
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
            icon.name: "dialog-cancel"
            displayHint: Kirigami.DisplayHint.AlwaysHide
            visible: feed && feed.status===Feed.Updating
            onTriggered: {
                feed.updater.abort()
            }
        }
    ]
}
