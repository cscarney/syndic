/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import org.kde.kirigami 2.14 as Kirigami
import com.rocksandpaper.syndic 1.0

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
            sortMode: globalSettings.feedListSort
            onRowsInserted: {
                if (feedList.currentIndex < 0) {
                    feedList.currentIndex = -1
                    currentlySelectedFeed = data(index(first,0), FeedListModel.FeedRole);
                }
            }
            onRowsRemoved: {
                feedList.currentIndex = 0;
                currentlySelectedFeed = data(index(0,0), FeedListModel.FeedRole);
            }
        }
        clip: true

        delegate: Kirigami.BasicListItem {
            id: listItem
            property var feed: model.feed
            property var iconName: feed ? feed.icon.toString() : ""
            property var status: feed ? feed.status : Feed.Idle
            property var unreadCount: feed ? feed.unreadCount : 0
            icon: iconName.length ? "image://feedicons/"+iconName : "feed-subscribe"
            label: feed ? feed.name : ""
            separatorVisible: false
            padding: Kirigami.Units.largeSpacing
            leftPadding: Kirigami.Units.smallSpacing
            rightPadding: Kirigami.Units.smallSpacing
            trailing: RowLayout {
                Kirigami.Icon {
                    source: "content-loading-symbolic"
                    Layout.preferredHeight: contentItem.height
                    isMask: true
                    visible: listItem.status === Feed.Updating
                }

                Kirigami.Icon {
                    source: "dialog-error-symbolic"
                    Layout.preferredHeight: contentItem.height
                    isMask: true
                    visible: listItem.status === Feed.Error
                }

                Label {
                    id: unreadCountLabel
                    visible: listItem.unreadCount !== 0
                    horizontalAlignment: Text.AlignCenter
                    Layout.rightMargin: background.radius * 2
                    text: listItem.unreadCount
                    leftInset: -background.radius
                    rightInset: -background.radius
                    background: Rectangle {
                        radius: parent.height / 2.0
                        color: Kirigami.Theme.activeBackgroundColor;
                    }
                }
            }

            onClicked: root.itemClicked()
        }/* delegate */

        section.property: "category"
        section.delegate: Kirigami.ListSectionHeader {
            label: section
            topPadding: padding * 2
            bottomPadding: padding * 2
        }

        move: Transition {
            enabled: root.visible
            NumberAnimation { properties: "x, y"; duration: Kirigami.Units.shortDuration }
        }

        displaced: Transition {
            enabled: root.visible
            NumberAnimation { properties: "x, y"; duration: Kirigami.Units.shortDuration }
        }

        onCurrentItemChanged: {
            if (currentItem)
                root.currentlySelectedFeed = currentItem.feed;
        }
    }
}
