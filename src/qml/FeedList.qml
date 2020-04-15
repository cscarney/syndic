import QtQuick 2.12
import QtQuick.Controls 2.12
import org.kde.kirigami 2.7 as Kirigami

ScrollView {
    id: root

    signal itemClicked
    signal feedSelected
    property alias currentItem: feedList.currentItem

    ScrollBar.vertical.policy: ScrollBar.AsNeeded
    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

    ListView {
        id: feedList
        anchors {
            fill: parent
            rightMargin: root.ScrollBar.vertical.visible ? root.ScrollBar.vertical.width : 0
        }

        model: feedManager.getFeedListModel()
        clip: true
        delegate: Kirigami.BasicListItem {
            separatorVisible: false
            label: isSpecial ? qsTr("All Items") : model.name
            property int feedId: model.id
            property string feedName: isSpecial ? qsTr("All Feeds") : model.name
            property bool isSpecial: model.id < 0
            onClicked: {
                feedList.currentIndex = index
                itemClicked()
            }
        }
        onCurrentItemChanged: {
            feedSelected();
        }
    }
}
