import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import org.kde.kirigami 2.7 as Kirigami
import Feed 1.0
import FeedListModel 1.0
import Enums 1.0

ScrollView {
    id: root
    property Feed currentlySelectedFeed
    property alias currentIndex: feedList.currentIndex
    signal itemClicked

    ScrollBar.vertical.policy: ScrollBar.AsNeeded
    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

    ListView {
        id: feedList

        anchors {
            fill: parent
            rightMargin: root.ScrollBar.vertical.visible ? root.ScrollBar.vertical.width : 0
        }

        currentIndex: 0
        model: FeedListModel{
            context: feedContext
            onRowsInserted: {
                feedList.currentIndex = -1;
                currentlySelectedFeed = data(index(first,0), FeedListModel.Feed);
            }
            onRowsRemoved: {
                feedList.currentIndex = 0;
                currentlySelectedFeed = data(index(0,0), FeedListModel.Feed);
            }
        }
        clip: true

        delegate: Kirigami.AbstractListItem {
            property color fgColor: highlighted ? Kirigami.Theme.highlightedTextColor : Kirigami.Theme.textColor
            property var feed: model.feed

            separatorVisible: false
            width: parent.width
            topPadding: Kirigami.Units.largeSpacing
            bottomPadding: Kirigami.Units.largeSpacing
            contentItem: RowLayout {
                property var feed: model.feed
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

            onClicked: {
                feedList.currentIndex = index;
                itemClicked();
            }
        }
        onCurrentItemChanged: {
            if (currentItem)
                root.currentlySelectedFeed = currentItem.feed;
        }
    }
}
