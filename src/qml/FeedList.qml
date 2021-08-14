/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import org.kde.kirigami 2.7 as Kirigami
import Feed 1.0
import FeedListModel 1.0


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
            rightMargin: root.ScrollBar.vertical.visible && !Kirigami.Settings.hasTransientTouchInput ? root.ScrollBar.vertical.width : 0
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

        delegate: Kirigami.BasicListItem {
            property var feed: model.feed
            property var iconName: feed.icon.toString()
            icon: iconName.length ? "image://feedicons/"+iconName : "feed-subscribe"
            label: feed.name
            separatorVisible: false
            padding: Kirigami.Units.largeSpacing
            leftPadding: Kirigami.Units.smallSpacing
            rightPadding: Kirigami.Units.smallSpacing
            trailing: RowLayout {
                property var feed: model.feed
                property var status: feed ? feed.status : Feed.Idle
                property var unreadCount: feed ? feed.unreadCount : 0

                Kirigami.Icon {
                    source: "content-loading-symbolic"
                    Layout.preferredHeight: contentItem.height
                    isMask: true
                    visible: parent.status === Feed.Updating
                }

                Kirigami.Icon {
                    source: "dialog-error-symbolic"
                    Layout.preferredHeight: contentItem.height
                    isMask: true
                    visible: parent.status === Feed.Error
                }

                Label {
                    id: unreadCountLabel
                    visible: parent.unreadCount !== 0
                    horizontalAlignment: Text.AlignCenter
                    Layout.rightMargin: background.radius * 2
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
