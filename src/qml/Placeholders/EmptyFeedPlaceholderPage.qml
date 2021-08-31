/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick 2.0
import org.kde.kirigami 2.7 as Kirigami

AbstractPlaceholderPage {
    property var model;

    iconName: "checkmark"
    headingText: "All Read"

    actions {
        main: Kirigami.Action {
            text: qsTr("Refresh Feed")
            iconName: "view-refresh"
            onTriggered: model.requestUpdate()
        }
    }
}
