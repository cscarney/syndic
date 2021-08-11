/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick 2.0
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtGraphicalEffects 1.0
import org.kde.kirigami 2.7 as Kirigami

RowLayout {
    id: root
    property alias text: messageLabel.text
    Kirigami.Theme.colorSet: Kirigami.Theme.Tooltip
    Kirigami.Theme.inherit: false

    Label {
        id: messageLabel
        property real inset: -Kirigami.Units.largeSpacing

        text: ""
        visible: (text) && (text!=="")
        color: Kirigami.Theme.textColor
        elide: Text.ElideRight
        topInset: inset
        bottomInset: inset
        leftInset: inset
        rightInset: inset

        Layout.maximumWidth: parent.width + inset * 4
        Layout.preferredWidth: implicitWidth
        Layout.alignment: Qt.AlignHCenter
        Layout.bottomMargin: - inset * 2.0

        background: Rectangle {
            color: Kirigami.Theme.backgroundColor
            radius: height / 2
            border.width: 1
            property color borderBase: Kirigami.Theme.textColor
            border.color: Qt.rgba(borderBase.r, borderBase.g, borderBase.b, 0.25);
        }
    }
}
