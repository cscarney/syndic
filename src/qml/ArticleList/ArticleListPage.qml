/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import com.rocksandpaper.syndic 1.0
import org.kde.kirigami 2.14 as Kirigami

AbstractArticleListPage {
    id: root

    actions {
        main: Kirigami.Action {
            text: qsTr("Mark All Read")
            iconName: "checkmark"
            visible: !clearReadAction.visible
            enabled: root.feed.unreadCount > 0
            onTriggered: {
                root.model.markAllRead();
            }
            displayHint: Kirigami.DisplayHint.KeepVisible
        }

        contextualActions: [
            Kirigami.Action {
                id: clearReadAction
                text: qsTr("Clear Read")
                iconName: "checkmark"
                visible: root.feed && root.unreadFilter && root.count > root.feed.unreadCount
                displayHint: Kirigami.DisplayHint.KeepVisible
                onTriggered: {
                    root.currentIndex = -1
                    root.model.removeRead()
                }
            },

            Kirigami.Action {
                 text: qsTr("Hide Read")
                 iconName: "view-filter"
                 checkable: true
                 checked: root.unreadFilter
                 displayHint: Kirigami.DisplayHint.AlwaysHide
                 onCheckedChanged: root.unreadFilter = checked
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
