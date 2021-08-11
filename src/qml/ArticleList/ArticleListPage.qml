/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import Feed 1.0
import org.kde.kirigami 2.14 as Kirigami

AbstractArticleListPage {
    id: root

    actions {
        main: Kirigami.Action {
            text: qsTr("Mark All Read")
            iconName: "mail-read"
            enabled: root.count > 0
            onTriggered: {
                root.model.markAllRead();
            }
            displayHint: Kirigami.DisplayHint.KeepVisible
        }

        contextualActions: [
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
