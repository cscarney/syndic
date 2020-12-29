import QtQuick 2.0
import org.kde.kirigami 2.7 as Kirigami
import QtQuick.Layouts 1.12

Item {
    id: root

    ColumnLayout {
        anchors {
            horizontalCenter: parent.horizontalCenter
            verticalCenter: parent.verticalCenter
        }

        Kirigami.Icon {
            id: theIcon
            source: "feedkeeper-feed-empty"
            Layout.minimumWidth: 32
            Layout.minimumHeight: 32
            Layout.preferredWidth: 128
            Layout.preferredHeight: 128
            Layout.alignment: Qt.AlignHCenter
            isMask: true;
            property var themeColor: Kirigami.Theme.textColor
            property var alpha: 0.5
            color: Qt.rgba(themeColor.r, themeColor.g, themeColor.b, alpha)
        }

        Kirigami.Heading {
            id: emptyText
            color: Kirigami.Theme.textColor
            horizontalAlignment: Text.AlignHCenter
            Layout.alignment: Qt.AlignHCenter
            text: qsTr("No Items");
        }
    }
}
