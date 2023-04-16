/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick 2.0
import org.kde.kirigami 2.14 as Kirigami
import com.rocksandpaper.syndic 1.0

AbstractFeedEditorPage {
    id: root
    required property Feed targetFeed;
    property var onDone: function(){}

    provisionalFeed: ProvisionalFeed {
        targetFeed: root.targetFeed
    }

    actions: [
        Kirigami.Action {
            text: qsTr("Save")
            iconName: "checkmark"
            onTriggered: {
                provisionalFeed.save();
                onDone();
            }
        },

        Kirigami.Action {
            text: qsTr("Delete")
            iconName: "delete"
            onTriggered: targetFeed.requestDelete();
            displayHint: Kirigami.DisplayHint.AlwaysHide
        }
    ]
}
