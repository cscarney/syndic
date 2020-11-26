import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import org.kde.kirigami 2.7 as Kirigami

ColumnLayout {
    id: root
    spacing: 5
    property color textColor: parent.highlighted ? Kirigami.Theme.highlightedTextColor : Kirigami.Theme.textColor;

    implicitHeight: headlineText.height + details.height + spacing

    Label {
        id: headlineText
        Layout.fillWidth: parent
        text: headline
        maximumLineCount: 2
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
        wrapMode: Text.WordWrap
        font.weight: isUnread ? Font.Bold : Font.ExtraLight
        color: textColor
        textFormat: Text.RichText
    }

    RowLayout {
        id: details
        Label {
            Layout.fillWidth: true
            text: author
            elide: Text.ElideRight
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            color: textColor
            font {
                italic: true
                weight: Font.Light
            }

            opacity: 0.6
        }

        Label {
            Layout.alignment: Qt.AlignRight
            text: Qt.formatDate(date)
            elide: Text.ElideRight
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignVCenter
            color: textColor
            opacity: 0.6
        }
    }
}
