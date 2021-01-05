import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import org.kde.kirigami 2.7 as Kirigami

Kirigami.Page {
    id: root
    property Item pageRow;

    title: qsTr("Add Content")

    ColumnLayout {
        anchors.fill: parent

        Kirigami.FormLayout {
            TextField {
                id: urlField
                Kirigami.FormData.label: qsTr("Feed URL")
            }
        }

        Button {
            Layout.alignment: Qt.AlignRight | Qt.AlignBottom
            action: Kirigami.Action {
                text: qsTr("OK")
                onTriggered: {
                    feedManager.addFeed(urlField.text)
                    pageRow && pageRow.clear()
                }
            }
        }
    }


}
