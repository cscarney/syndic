/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick 2.0
import org.kde.kirigami 2.7 as Kirigami
import QtQuick.Layouts 1.12

Item {
    id: root
    property alias icon: theIcon.source
    property alias text: emptyText.text

    ColumnLayout {
        anchors {
            horizontalCenter: parent.horizontalCenter
            verticalCenter: parent.verticalCenter
        }

        Kirigami.Icon {
            id: theIcon
            Layout.minimumWidth: 32
            Layout.minimumHeight: 32
            Layout.preferredWidth: 128
            Layout.preferredHeight: 128
            Layout.alignment: Qt.AlignHCenter
            isMask: true;
            property color themeColor: Kirigami.Theme.textColor
            property real alpha: 0.5
            color: Qt.rgba(themeColor.r, themeColor.g, themeColor.b, alpha)
        }

        Kirigami.Heading {
            id: emptyText
            color: Kirigami.Theme.textColor
            horizontalAlignment: Text.AlignHCenter
            Layout.alignment: Qt.AlignHCenter
        }
    }
}
