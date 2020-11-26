import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import Qt.labs.settings 1.1
import org.kde.kirigami 2.7 as Kirigami


ColumnLayout {
    id: mainLayout

    Settings {
        category: "ArticlePage"
        property alias fontSize: contentTextEdit.font.pointSize
    }

    property var item
    property var firstImage: ""
    property var textWithoutImages: ""

    readonly property string textStyle: "<style>
    * {
        line-height: 150%;
    }
    .image-wrapper,img {
        line-height: 100%
    }
    p {
        text-indent: 10px;
        padding-bottom: 12px;
        font-family: serif;
    }
    </style>"

    Kirigami.Heading {
        level: 1
        Layout.fillWidth: true
        text: item.headline
        elide: Text.ElideRight
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        wrapMode: Text.Wrap
        color: Kirigami.Theme.textColor
        textFormat: Text.RichText


        MouseArea {
            id: titleMouse
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onPressed: Qt.openUrlExternally(item.url)
        }
    }

    RowLayout {
        Label {
            Layout.fillWidth: true
            text: item.author
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
            text: Qt.formatDate(item.date)
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
        selectByMouse: true
        color: Kirigami.Theme.textColor
        selectedTextColor: Kirigami.Theme.highlightedTextColor
        selectionColor: Kirigami.Theme.highlightColor

        onLinkActivated: function(link){
            Qt.openUrlExternally(link)
        }
        MouseArea
        {
            id: contentMouse
            anchors.fill: parent
            cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.IBeamCursor
            onPressed:  mouse.accepted = false
            onWheel: {
                    if (wheel.modifiers & Qt.ControlModifier) {
                        parent.font.pointSize += (wheel.angleDelta.y/Math.abs(wheel.angleDelta.y))
                    } else {
                        wheel.accepted = false
                    }
            }
        }
    }
}
