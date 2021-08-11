/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick 2.0
import org.kde.kirigami 2.7 as Kirigami

AbstractFeedEditorPage {
    property bool keepDrawerOpen: true
    title: qsTr("Add Content")

    actions {
        main: Kirigami.Action {
            id: previewAction
            text: qsTr("Preview...")
            iconName: "document-preview"
            checkable: true
            checked: previewOpen
            onToggled: previewOpen = checked
        }
    }

    Keys.onReturnPressed: previewOpen = true
    Keys.onEnterPressed: previewOpen = true
}
