/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import org.kde.kirigami 2.7 as Kirigami

ColumnLayout {
    id: root
    property color textColor: parent.highlighted ? Kirigami.Theme.highlightedTextColor : Kirigami.Theme.textColor;

    spacing: 5
    implicitHeight: headlineText.height + details.height + spacing

    Label {
        id: headlineText
        Layout.fillWidth: parent
        text: ref.article.title
        maximumLineCount: 2
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
        wrapMode: Text.WordWrap
        font {
            weight: ref.article.isRead ? Font.ExtraLight : Font.Bold
            pointSize: Kirigami.Theme.defaultFont.pointSize
        }
        color: textColor
        textFormat: Text.StyledText
    }

    RowLayout {
        id: details
        Label {
            Layout.fillWidth: true
            text: ref.article.author
            elide: Text.ElideRight
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            color: textColor
            font {
                pointSize: Kirigami.Theme.smallFont.pointSize
                italic: true
                weight: Font.Light
            }

            opacity: 0.6
        }

        Label {
            Layout.alignment: Qt.AlignRight
            text: Qt.formatDate(ref.article.date)
            elide: Text.ElideRight
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignVCenter
            color: textColor
            opacity: 0.6
            font.pointSize: Kirigami.Theme.smallFont.pointSize
        }
    }
}
