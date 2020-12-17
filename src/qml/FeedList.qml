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
            property color fgColor: highlighted ? Kirigami.Theme.highlightedTextColor : Kirigami.Theme.textColor

            separatorVisible: false
            width: parent.width
            topPadding: Kirigami.Units.largeSpacing
            bottomPadding: Kirigami.Units.largeSpacing
            contentItem: RowLayout {
                property var feed: model.feedRef.feed
                property var status: feed ? feed.status : Enums.Idle
                property var unreadCount: feed ? feed.unreadCount : 0
                property var name: feed ? feed.name : qsTr("All Items")

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
                    text: parent.name
                    textFormat: Text.StyledText
                    color: fgColor
                    font.weight: parent.unreadCount !== 0 ? Font.Black : Font.Light
                }

                Kirigami.Icon {
                    Layout.alignment: Qt.AlignRight
                    Layout.preferredHeight: feedNameLabel.implicitHeight
                    Layout.preferredWidth: feedNameLabel.implicitHeight
                    source: "content-loading-symbolic"
                    isMask: true
                    color: fgColor
                    visible: parent.status === Enums.Updating
                }

                Kirigami.Icon {
                    Layout.alignment: Qt.AlignRight
                    Layout.preferredHeight: feedNameLabel.implicitHeight
                    Layout.preferredWidth: feedNameLabel.implicitHeight
                    source: "dialog-error-symbolic"
                    isMask: true
                    color: fgColor
                    visible: parent.status === Enums.Error
                }

                Label {
                    id: unreadCountLabel
                    visible: parent.unreadCount !== 0
                    Layout.alignment: Qt.AlignRight
                    Layout.leftMargin: Kirigami.Units.smallSpacing
                    Layout.rightMargin: Kirigami.Units.smallSpacing
                    Layout.minimumWidth: implicitBackgroundWidth
                    Layout.minimumHeight: feedNameLabel.implicitHeight
                    horizontalAlignment: Text.AlignCenter
                    color: Kirigami.Theme.activeTextColor
                    text: parent.unreadCount
                    leftInset: -background.radius
                    rightInset: -background.radius
                    background: Rectangle {
                        radius: parent.height / 2.0
                        color: Kirigami.Theme.activeBackgroundColor;
                    }
                }
            }

            property var feedRef: model.feedRef
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
