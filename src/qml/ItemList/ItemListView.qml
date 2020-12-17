import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import org.kde.kirigami 2.9 as Kirigami
import Enums 1.0

ListView {
    id: articleList
    clip: true

    delegate: Kirigami.AbstractListItem {
        anchors {
            left: parent.left
            right: parent.right
        }

        text: ref.article.headline
        padding: 10
        contentItem: ItemListEntry { }
        
        property var data: ref

        highlighted: ListView.isCurrentItem
        onClicked: {
            articleList.currentIndex = model.index
        }
    }

    header: Rectangle {
        visible: model.status === Enums.Updating
        height: visible ? updatingText.implicitHeight * 3 : 0
        width: parent.width
        color: Kirigami.Theme.backgroundColor

        RowLayout {
            anchors {
                horizontalCenter: parent.horizontalCenter
                verticalCenter: parent.verticalCenter
            }
            BusyIndicator {
                running: model.status === Enums.Updating
            }
            Label {
                id: updatingText
                color: Kirigami.Theme.neutralTextColor
                Layout.alignment: Qt.AlignVCenter
            }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: articleList.currentIndex = -1
        }
    }
}
