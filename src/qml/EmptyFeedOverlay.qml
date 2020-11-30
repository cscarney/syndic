import QtQuick 2.0
import org.kde.kirigami 2.7 as Kirigami

Rectangle {
    color: Kirigami.Theme.backgroundColor
    Kirigami.Heading {
        id: emptyText
        anchors {
            horizontalCenter: parent.horizontalCenter
            verticalCenter: parent.verticalCenter
        }
        color: Kirigami.Theme.textColor
        text: qsTr("Nothing to Show")
    }
}
