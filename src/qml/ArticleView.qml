import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import Qt.labs.settings 1.1
import org.kde.kirigami 2.7 as Kirigami
import Article 1.0

ColumnLayout {
    id: root
    property var item
    property string firstImage: ""
    property string textWithoutImages: ""
    property string hoveredLink: titleMouse.containsMouse ? item.article.url : contentTextEdit.hoveredLink

    readonly property string textStyle: "<style>
    * {
        line-height: 150%;
    }
    img {
        line-height: 100%
    }
    p {
        padding-bottom: 12px;
        font-family: serif;
    }
    </style>"

    Settings {
        // @disable-check M16
        category: "ArticlePage"
        property alias fontSize: contentTextEdit.font.pointSize
    }

    Kirigami.Heading {
        level: 1
        Layout.fillWidth: true
        text: item.article.title || "(no title)"
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
        }
    }

    RowLayout {
        Label {
            Layout.fillWidth: true
            text: item.article.author || "(no author)"
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

    Image {
        visible: firstImage!==""
        source: firstImage
        property real scaleToFit: Math.min(parent.width, sourceSize.width) / sourceSize.width
        Layout.preferredWidth: sourceSize.width * scaleToFit
        Layout.preferredHeight: sourceSize.height * scaleToFit
        Layout.alignment: Qt.AlignHCenter
        fillMode: Image.PreserveAspectFit
    }

    TextEdit {
        id: contentTextEdit
        Layout.fillWidth: true
        Layout.topMargin: 20
        textFormat: Text.RichText
        text: textStyle + textWithoutImages
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignTop
        wrapMode: Text.Wrap
        readOnly: true
        selectByMouse: contentMouse.isRealMouse
        color: Kirigami.Theme.textColor
        selectedTextColor: Kirigami.Theme.highlightedTextColor
        selectionColor: Kirigami.Theme.highlightColor

        onLinkActivated: function(link){
            Qt.openUrlExternally(link)
        }

        PinchArea {
            id: contentPinch
            anchors.fill: parent
            property real startingSize
            property int anchorPosition
            readonly property real minFontSize: 6.0
            readonly property real maxFontSize: 72.0

            onPinchStarted: {
                startingSize = parent.font.pointSize;
                var startCenter = pinch.startCenter;
                anchorPosition = parent.positionAt(startCenter.x, startCenter.y);
            }

            onPinchUpdated: {
                parent.font.pointSize = clamp(startingSize * pinch.scale, minFontSize, maxFontSize);
                positionAnchor(pinch.center.y);
            }

            onPinchFinished: {
                scroller.returnToBounds();
            }

            function positionAnchor(targetY) {
                var anchorRect = parent.positionToRectangle(anchorPosition);
                var anchorY = anchorRect.y + anchorRect.height / 2.0;
                var delta = anchorY - targetY;
                scroller.contentY += delta;  // FIXME `scroller` is a non-local reference to ArticlePage.qml
            }

            function clamp(num, min, max) {
              return num <= min ? min : num >= max ? max : num;
            }

            MouseArea
            {
                id: contentMouse
                property bool isRealMouse: false
                anchors.fill: parent
                cursorShape: contentTextEdit.hoveredLink ? Qt.PointingHandCursor : Qt.IBeamCursor
                onPressed: {
                    isRealMouse = (mouse.source === Qt.MouseEventNotSynthesized)
                    mouse.accepted = false
                }
                onWheel: {
                        if (wheel.modifiers & Qt.ControlModifier) {
                            contentTextEdit.font.pointSize += Math.sign(wheel.angleDelta.y)
                        } else {
                            wheel.accepted = false
                        }
                }
            }
        }
    }
}
