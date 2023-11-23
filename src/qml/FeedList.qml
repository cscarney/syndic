/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import org.kde.kirigami 2.14 as Kirigami
import org.kde.kirigami.delegates as Delegates
import com.rocksandpaper.syndic 1.0

ListView {
    id: root
    property Feed currentlySelectedFeed
    signal itemClicked

    currentIndex: -1
    clip: true
    pressDelay: Kirigami.Settings.hasTransientTouchInput ? 120 : 0
    highlightMoveDuration: Kirigami.Units.longDuration

    model: FeedListModel{
        id: feedListModel
        context: feedContext
        sortMode: globalSettings.feedListSort

        onRowsRemoved: {
            root.currentIndex = 0;
        }
    }

    delegate: ItemDelegate {
        id: listItem
        required property int index
        required property var feed
        property int status: feed.status
        property int unreadCount: feed.unreadCount
        width: root.width
        verticalPadding: Kirigami.Units.largeSpacing
        highlighted: ListView.isCurrentItem

        contentItem: RowLayout {
            spacing: Kirigami.Units.smallSpacing

            FeedIcon {
                feed: listItem.feed
            }

            Delegates.TitleSubtitle {
                Layout.fillWidth: true
                title: feed.name
                selected: listItem.highlighted
            }

            Kirigami.Icon {
                source: "content-loading-symbolic-nomask"
                Layout.preferredHeight: contentItem.height
                visible: listItem.status === Feed.Updating
                selected: listItem.highlighted
            }

            Kirigami.Icon {
                source: "dialog-error-symbolic-nomask"
                Layout.preferredHeight: contentItem.height
                visible: listItem.status === Feed.Error
                selected: listItem.highlighted
            }

            Label {
                id: unreadCountLabel
                visible: listItem.unreadCount !== 0
                horizontalAlignment: Text.AlignCenter
                Layout.leftMargin: background.radius
                Layout.rightMargin: background.radius
                text: listItem.unreadCount
                leftInset: -background.radius
                rightInset: -background.radius
                background: Rectangle {
                    radius: parent.height / 2.0
                    color: Kirigami.Theme.activeBackgroundColor;
                }
            }
        }

        onClicked: {
            ListView.view.currentIndex = index;
            root.itemClicked()
        }
    }/* delegate */

    section.property: "category"
    section.delegate: Kirigami.ListSectionHeader {
        required property string section
        label: section
        topPadding: padding * 2
        bottomPadding: padding * 2
        width: parent?.width > 0 ? parent.width : implicitWidth
        onClicked: {
            currentIndex = -1
            root.currentlySelectedFeed = feedContext.createCategoryFeed(section);
            root.itemClicked();
        }
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

    function selectFeed(feed) {
        root.forceLayout()
        root.currentIndex = feedListModel.indexOf(feed)
    }
}
