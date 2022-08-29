/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick 2.0
import org.kde.kirigami 2.7 as Kirigami

AbstractPlaceholderPage {
    property var model;

    iconName: "dialog-error-symbolic-nomask"
    headingText: "Error Updating Feed"
    descriptionText: "This feed is currently offline"

    actions {
        main: Kirigami.Action {
            text: qsTr("Retry")
            iconName: "view-refresh"
            onTriggered: model.requestUpdate()
        }
    }
}
