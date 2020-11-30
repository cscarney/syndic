import QtQuick 2.0
import org.kde.kirigami 2.7 as Kirigami
import ItemModel 1.0

Rectangle {
    id: root
    property var status;
    property bool unreadFilter: false;

    color: Kirigami.Theme.backgroundColor
    Kirigami.Heading {
        id: emptyText
        anchors {
            horizontalCenter: parent.horizontalCenter
            verticalCenter: parent.verticalCenter
        }
        color: Kirigami.Theme.textColor
        text: {
            switch (status) {
            case ItemModel.Ok:
                return unreadFilter ? qsTr("All Read") : qsTr("Empty Feed");
            case ItemModel.Loading:
                return qsTr("Loading Feed");
            case ItemModel.Updating:
                return qsTr("Updating Feed");
            case ItemModel.Error:
                return qsTr("Error Updating Feed");
            default:
                return qsTr("Nothing to Show");
            }
        }
    }
}
