import QtQuick 2.12
import QtQuick.Layouts 1.12
import org.kde.kirigami 2.7 as Kirigami

Kirigami.GlobalDrawer {
     id: root

     signal feedSelected
     property alias currentItem: feedList.currentItem

    handleVisible: modal && isFirstPage

    Kirigami.Theme.colorSet: Kirigami.Theme.View

    topContent: ColumnLayout {
        spacing: 0
        Layout.fillWidth: true

        FeedList {
            id: feedList
            Layout.fillHeight: true
            Layout.fillWidth: true
            implicitHeight: root.height
            onFeedSelected: root.feedSelected()
            onItemClicked: root.drawerOpen = !root.modal
        }

        Kirigami.Separator {
            Layout.fillWidth: true
            Layout.leftMargin: -root.leftPadding
            Layout.rightMargin: -root.rightPadding
        }
    }
}
