/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick 2.0
import org.kde.kirigami 2.7 as Kirigami
import com.rocksandpaper.syndic 1.0

AbstractFeedEditorPage {
    property bool keepDrawerOpen: true
    title: qsTr("Add Content")

    provisionalFeed: ProvisionalFeed {
        updateMode: Feed.InheritUpdateMode
        updateInterval: globalSettings.updateInterval
        expireMode: Feed.InheritUpdateMode
        expireAge: globalSettings.expireAge
    }

    actions {
        main: Kirigami.Action {
            id: previewAction
            text: qsTr("Preview…")
            iconName: "document-preview"
            checkable: true
            checked: previewOpen
            onToggled: previewOpen = checked
        }
    }

    Keys.onReturnPressed: previewOpen = true
    Keys.onEnterPressed: previewOpen = true
}
