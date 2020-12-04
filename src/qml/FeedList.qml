import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import org.kde.kirigami 2.7 as Kirigami
import FeedListModel 1.0
import Enums 1.0

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

        currentIndex: 0
        model: FeedListModel{ manager: feedManager }
        clip: true

        delegate: Kirigami.AbstractListItem {
            separatorVisible: false
            width: parent.width
            topPadding: Kirigami.Units.largeSpacing
            bottomPadding: Kirigami.Units.largeSpacing
            contentItem: RowLayout {
                Kirigami.Icon {
                    source: model.icon
                    Layout.minimumHeight: feedNameLabel.height
                    Layout.minimumWidth: feedNameLabel.height
                }

                Label {
                    id: feedNameLabel
                    horizontalAlignment: Text.AlignLeft
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                    text: model.name
                    font.weight: model.unreadCount !== 0 ? Font.Black : Font.Light
                }

                Kirigami.Icon {
                    Layout.alignment: Qt.AlignRight
                    Layout.preferredHeight: feedNameLabel.implicitHeight
                    Layout.preferredWidth: feedNameLabel.implicitHeight
                    source: "view-refresh"
                    visible: model.status === Enums.Updating
                }

                Kirigami.Icon {
                    Layout.alignment: Qt.AlignRight
                    Layout.preferredHeight: feedNameLabel.implicitHeight
                    Layout.preferredWidth: feedNameLabel.implicitHeight
                    source: "error"
                    visible: model.status === Enums.Error
                }

                Label {
                    id: unreadCountLabel
                    visible: model.unreadCount !== 0
                    Layout.alignment: Qt.AlignRight
                    Layout.leftMargin: Kirigami.Units.smallSpacing
                    Layout.rightMargin: Kirigami.Units.smallSpacing
                    Layout.minimumWidth: implicitBackgroundWidth
                    Layout.minimumHeight: feedNameLabel.implicitHeight
                    horizontalAlignment: Text.AlignCenter
                    color: Kirigami.Theme.activeTextColor
                    text: model.unreadCount
                    leftInset: -background.radius
                    rightInset: -background.radius
                    background: Rectangle {
                        radius: parent.height / 2.0
                        color: Kirigami.Theme.activeBackgroundColor;
                    }
                }
            }

            property int feedId: model.id
            property string feedName: model.name
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
