/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import com.rocksandpaper.syndic 1.0
import org.kde.kirigami 2.14 as Kirigami

AbstractArticleListPage {
    id: root
    unreadFilter: globalSettings.unreadFilter

    actions {
        main: Kirigami.Action {
            text: qsTr("Mark All Read")
            iconName: "checkmark"
            enabled: root.feed && root.feed.unreadCount > 0
            onTriggered: {
                root.model.markAllRead();
            }
            displayHint: Kirigami.DisplayHint.KeepVisible
        }

        right: Kirigami.Action {
            id: clearReadAction
            text: qsTr("Clear")
            iconName: "edit-clear-all"
            visible: root.feed && root.unreadFilter
            enabled: root.feed && root.count > root.feed.unreadCount
            onTriggered: {
                root.currentIndex = -1
                root.model.removeRead()
            }
        }

        contextualActions: [
            Kirigami.Action {
                 text: qsTr("Hide Read")
                 iconName: "view-filter"
                 checkable: true
                 checked: globalSettings.unreadFilter
                 displayHint: Kirigami.DisplayHint.AlwaysHide
                 onCheckedChanged: globalSettings.unreadFilter = checked
             },

            Kirigami.Action {
                text: qsTr("Edit...")
                iconName: "document-edit"
                displayHint: Kirigami.DisplayHint.AlwaysHide
                visible: feed && feed.editable
                onTriggered: {
                    pageRow.pop(root)
                    pageRow.push("qrc:/qml/EditFeedPage.qml", {targetFeed: feed});
                }
            },

            Kirigami.Action {
                text: qsTr("Refresh")
                iconName: "view-refresh"
                displayHint: Kirigami.DisplayHint.AlwaysHide
                enabled: feed && feed.status!=Feed.Updating
                onTriggered: {
                    root.model.requestUpdate();
                }
            }
        ]
    }
}
