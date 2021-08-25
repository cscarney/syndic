/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import Qt.labs.settings 1.1
import org.kde.kirigami 2.7 as Kirigami
import Article 1.0
import ContentModel 1.0
import ContentImage 1.0

ColumnLayout {
    id: root
    property var item
    property string text: ""
    property string hoveredLink

    readonly property string textStyle: "<style>
    * {
        line-height: 150%;
    }
    p {
        padding-bottom: 12px;
    }
    </style>"

    Settings {
        id: articleViewSettings
        category: "ArticlePage"
        property real fontSize
    }

    Kirigami.Heading {
        property string linkText: item.article.url
        level: 1
        Layout.fillWidth: true
        text: item.article.title
        elide: Text.ElideRight
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        wrapMode: Text.Wrap
        color: Kirigami.Theme.textColor
        textFormat: Text.RichText

        MouseArea {
            id: titleMouse
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onPressed: Qt.openUrlExternally(item.article.url)
            onContainsMouseChanged: root.hoveredLink = containsMouse ? item.article.url : null
        }
    }

    RowLayout {
        Label {
            Layout.fillWidth: true
            text: item.article.author + ", " + item.article.feed.name
            elide: Text.ElideRight
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            color: Kirigami.Theme.textColor
            opacity: 0.6
            font {
                italic: true
                weight: Font.Light
            }
        }

        Label {
            Layout.alignment: Qt.AlignRight
            text: Qt.formatDate(item.article.date)
            elide: Text.ElideRight
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignVCenter
            color: Kirigami.Theme.textColor
            opacity: 0.6
            font {
                italic: true
                weight: Font.Light
            }
        }
    }

    Repeater {
        model: ContentModel {
            text: root.text
        }

        delegate: Loader {
            Layout.fillWidth: item.Layout.fillWidth
            Layout.alignment: item.Layout.alignment
            Layout.preferredHeight: item.Layout.preferredHeight
            Layout.preferredWidth: item.Layout.preferredWidth
            Layout.topMargin: item.Layout.topMargin
            sourceComponent: switch (block.delegateName) {
                  case "ImageBlock":
                      return imageBlockComponent;
                  case "TextBlock":
                      return textBlockComponent;
                  default:
                      return undefined;
            }
            property var modelBlock: block
        }
    }

    Component {
        id: imageBlockComponent
        ContentImage {
            property string href: modelBlock.resolvedHref(root.item.article.url);
            property real scaleToFit: Math.min(root.width, implicitWidth) / implicitWidth
            source: modelBlock.resolvedSrc(root.item.article.url);
            Layout.preferredWidth: implicitWidth * scaleToFit
            Layout.preferredHeight: implicitHeight * scaleToFit
            Layout.alignment: Qt.AlignHCenter

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: href ? Qt.PointingHandCursor : Qt.ArrowCursor
                onContainsMouseChanged: root.hoveredLink = containsMouse ? href : null
                onClicked: Qt.openUrlExternally(href)
            }
        }
    }

    Component {
        id: textBlockComponent
        TextEdit {
            id: contentTextEdit
            property string linkText: hoveredLink
            Layout.fillWidth: true
            Layout.topMargin: 20
            textFormat: Text.RichText
            text: textStyle + modelBlock.text
            font.pointSize: articleViewSettings.fontSize
            font.family: "serif"
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignTop
            wrapMode: Text.Wrap
            readOnly: true
            selectByMouse: !Kirigami.Settings.hasTransientTouchInput
            color: Kirigami.Theme.textColor
            selectedTextColor: Kirigami.Theme.highlightedTextColor
            selectionColor: Kirigami.Theme.highlightColor
            baseUrl: root.item.article.url

            onLinkActivated: function(link){
                Qt.openUrlExternally(root.item.article.resolvedLink(link))
            }

            onHoveredLinkChanged: root.hoveredLink = hoveredLink ? root.item.article.resolvedLink(hoveredLink) : ""

            MouseArea
            {
                id: contentMouse
                anchors.fill: parent
                cursorShape: contentTextEdit.hoveredLink ? Qt.PointingHandCursor : Qt.IBeamCursor
                scrollGestureEnabled: true
                acceptedButtons: Qt.NoButton
                onWheel: {
                        if (wheel.modifiers & Qt.ControlModifier) {
                            articleViewSettings.fontSize += Math.sign(wheel.angleDelta.y)
                        } else {
                            wheel.accepted = false
                        }
                }
            }
        }
    }
}
