/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import com.rocksandpaper.syndic 1.0
import org.kde.kirigami 2.14 as Kirigami

AbstractArticleListPage {
    id: root
    automaticOpen: false
    unreadFilter: false

    actions {
        main: Kirigami.Action {
            id: saveAction
            text: qsTr("Subscribe")
            iconName: "list-add"
            enabled: model.status == Feed.Idle
            onTriggered: {
                var feed = root.feed
                pageRow.clear();
                feedContext.addFeed(feed);
            }
        }
    }
}
