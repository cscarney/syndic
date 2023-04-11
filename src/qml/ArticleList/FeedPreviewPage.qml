/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import com.rocksandpaper.syndic 1.0
import QtQuick.Window 2.15
import org.kde.kirigami 2.14 as Kirigami

AbstractArticleListPage {
    id: root
    required property ProvisionalFeed provisionalFeed
    automaticOpen: false
    unreadFilter: false
    feed: provisionalFeed

    actions {
        main: Kirigami.Action {
            id: saveAction
            text: qsTr("Subscribe")
            iconName: "list-add"
            enabled: model.status == Feed.Idle
            onTriggered: {
                const provisionalFeed = root.provisionalFeed
                feedContext.addFeed(provisionalFeed);
                provisionalFeed.targetFeedChanged.connect(()=>{
                    const w = root.Window.window;
                    if (w && w.selectFeed) {
                        w.selectFeed(provisionalFeed.targetFeed);
                    }
                });
            }
        }
    }
}
