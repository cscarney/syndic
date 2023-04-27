/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Window 2.15
import org.kde.kirigami 2.7 as Kirigami
import com.rocksandpaper.syndic 1.0

ColumnLayout {
    id: root
    required property Article article
    property string text: ""
    property string hoveredLink
    property bool showExpandedByline: false

    readonly property string textStyle: "<style>
    * {
        line-height: 150%;
    }
    p {
        padding-bottom: 12px;
    }
    </style>"

    Kirigami.Heading {
        level: 1
        Layout.fillWidth: true
        text: root.article.title
        elide: Text.ElideRight
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        wrapMode: Text.Wrap
        color: Kirigami.Theme.textColor
        textFormat: Text.RichText

        MouseArea {
            id: titleMouse
            anchors.fill: parent
            hoverEnabled: !Kirigami.Settings.hasTransientTouchInput
            cursorShape: Qt.PointingHandCursor
            onClicked: Qt.openUrlExternally(root.article.url)
            onContainsMouseChanged: root.hoveredLink = containsMouse ? root.article.url : null
        }
    }

    RowLayout {
        Label {
            property string expandedByline: root.article.author + ", <a href=\"syndic-feed-link:\">" + root.article.feed.name + "</a>"
            Layout.fillWidth: true
            text: showExpandedByline ? expandedByline : root.article.author
            elide: Text.ElideRight
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            color: Kirigami.Theme.textColor
            opacity: 0.6
            font {
                italic: true
                weight: Font.Light
            }

            onLinkActivated: (link)=>{
                const w = Window.window;
                if (w && w.selectFeed) {
                    w.selectFeed(root.article.feed);
                }
            }
        }

        Label {
            Layout.alignment: Qt.AlignRight
            text: Qt.formatDate(root.article.date)
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
        id: contentRepeater

        model: ContentModel {
            text: root.text
        }

        delegate: Loader {
            id: loader
            required property int index
            required property var model
            property var modelBlock: model.block
            Layout.fillWidth: loader.item.Layout.fillWidth
            Layout.alignment: loader.item.Layout.alignment
            Layout.preferredHeight: loader.item.Layout.preferredHeight
            Layout.preferredWidth: loader.item.Layout.preferredWidth
            Layout.topMargin: loader.item.Layout.topMargin

            // Load blocks asynchronously, in order, hiding them until they're ready
            asynchronous: true
            active: index===0 || contentRepeater.itemAt(index-1).status===Loader.Ready
            visible: status===Loader.Ready

            sourceComponent: switch (loader.modelBlock.delegateName) {
                  case "ImageBlock":
                      return imageBlockComponent;
                  case "TextBlock":
                      return textBlockComponent;
                  default:
                      return undefined;
            }
        }
    }

    Component {
        id: imageBlockComponent

        Item {
            id: imageBlock
            property string href: modelBlock.resolvedHref(root.article.url);
            property real scaleToFit: Math.min(root.width, implicitWidth) / implicitWidth

            implicitWidth: modelBlock.size.width || modelBlock.sizeGuess.width || Kirigami.Units.iconSizes.small * 4
            implicitHeight: modelBlock.size.height || (implicitWidth / modelBlock.aspectRatio)

            Layout.preferredWidth: implicitWidth * scaleToFit
            Layout.preferredHeight: implicitHeight * scaleToFit
            Layout.alignment: Qt.AlignHCenter
            ToolTip.text: modelBlock.title ?? ""
            ToolTip.visible: (ToolTip.text != "") && (imageMouse.containsMouse || imageMouse.pressed)
            ToolTip.delay: Kirigami.Units.toolTipDelay
            ContentImage {
                id: image
                anchors.fill: parent
                source: modelBlock.resolvedSrc(root.article.url);
                opacity: 0
            }

            Item {
                id: loadingImagePlaceholder
                anchors.fill: parent

                Rectangle {
                    anchors.fill: parent
                    color: "transparent"
                    border.color: Kirigami.Theme.textColor
                    border.width: 1
                }

                Kirigami.Icon {
                    source: image.loadStatus===ContentImage.Loading ? "content-loading-symbolic-nomask" : "dialog-error-symbolic-nomask"
                    anchors.centerIn: parent
                    height: Kirigami.Units.iconSizes.small
                    width: height
                }
            }

            MouseArea {
                id: imageMouse
                anchors.fill: parent
                hoverEnabled: !Kirigami.Settings.hasTransientTouchInput
                cursorShape: href ? Qt.PointingHandCursor : Qt.ArrowCursor
                onContainsMouseChanged: root.hoveredLink = containsMouse ? href : null
                onClicked: Qt.openUrlExternally(href)
            }

            /* Skip the load transition if the image loads quickly enough
               to be percieved as part of the page loading. */
            Timer {
                id: longLoadTimer
                interval: 250
                running: true
            }

            states: [
                State {
                    name: "imageLoaded"
                    when: image.loadStatus===ContentImage.Complete
                    PropertyChanges {
                        target: imageBlock
                        implicitWidth: modelBlock.size.width || image.implicitWidth
                        implicitHeight: modelBlock.size.width ? modelBlock.size.width * (image.implicitHeight/image.implicitWidth) : image.implicitHeight
                    }
                    PropertyChanges {
                        target: image
                        opacity: 1
                    }
                    PropertyChanges {
                        target: loadingImagePlaceholder
                        visible: false
                    }
                }

            ]

            transitions: [
                Transition {
                    to: "imageLoaded"
                    enabled: !longLoadTimer.running
                    NumberAnimation {
                        properties: "implicitWidth,implicitHeight,opacity"
                        duration: Kirigami.Units.shortDuration
                        easing.type: Easing.OutCubic
                    }
                }

            ]
        }
    }

    Component {
        id: textBlockComponent
        TextEdit {
            id: contentTextEdit
            Layout.fillWidth: true
            Layout.topMargin: 20
            textFormat: Text.RichText
            text: textStyle + modelBlock.text
            font.pointSize: Math.max(fontMetrics.font.pointSize + globalSettings.textAdjust, 6)
            font.family: globalSettings.bodyFont || Kirigami.Theme.defaultFont.family
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignTop
            wrapMode: Text.Wrap
            readOnly: true
            selectByMouse: !Kirigami.Settings.hasTransientTouchInput
            color: Kirigami.Theme.textColor
            selectedTextColor: Kirigami.Theme.highlightedTextColor
            selectionColor: Kirigami.Theme.highlightColor
            baseUrl: root.article.url

            // native rendering is glitchy with selection highlight
            renderType: Text.QtRendering

            onLinkActivated: function(link){
                Qt.openUrlExternally(root.article.resolvedLink(link))
            }

            // The root hover link gets the most recently changed value over all the views.
            // We ignore hover link changes caused by touch events, since TextEdit::hoveredLink
            // *only* responds to touch input in degenerate cases.
            onHoveredLinkChanged: root.hoveredLink = hoveredLink && !Kirigami.Settings.hasTransientTouchInput ? root.article.resolvedLink(hoveredLink) : ""

            MouseArea
            {
                id: contentMouse
                anchors.fill: parent
                cursorShape: contentTextEdit.hoveredLink ? Qt.PointingHandCursor : Qt.IBeamCursor
                scrollGestureEnabled: true
                acceptedButtons: Qt.NoButton
                onWheel: function(wheel){
                    if (wheel.modifiers & Qt.ControlModifier) {
                        globalSettings.textAdjust += Math.sign(wheel.angleDelta.y);
                    } else {
                        wheel.accepted = false
                    }
                }
            }
        }
    }

    FontMetrics{
        id: fontMetrics
    }
}
