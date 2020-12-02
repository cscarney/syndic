import QtQuick 2.0
import org.kde.kirigami 2.7 as Kirigami
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import ItemModel 1.0

Kirigami.Page {
    id: root

    property string iconName: ""
    property string headingText: ""
    property string descriptionText: qsTr("Select an item to view it here")

    Kirigami.Theme.colorSet: Kirigami.Theme.View
    ColumnLayout {
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter:  parent.horizontalCenter
        Kirigami.Icon {
            id: statusIcon
            source: iconName
            visible: iconName != ""
            Layout.minimumWidth: 32
            Layout.minimumHeight: 32
            Layout.preferredWidth: 128
            Layout.preferredHeight: 128
            Layout.alignment: Qt.AlignHCenter
            isMask: true;
            color: Kirigami.Theme.textColor
        }
        Kirigami.Heading {
            id: heading
            color: Kirigami.Theme.textColor
            horizontalAlignment: Text.AlignHCenter
            Layout.alignment: Qt.AlignHCenter
            text: headingText
            visible: headingText != ""
        }

        Label {
            id: description
            Layout.alignment: Qt.AlignCenter
            text: descriptionText
        }
    }
}
