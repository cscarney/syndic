/**
 * SPDX-FileCopyrightText: 2025 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import org.kde.kirigami 2.12 as Kirigami
import com.rocksandpaper.syndic 1.0

Kirigami.Dialog {
    id: root
    
    title: qsTr("Select feeds to display web content:")
    
    required property var feedListModel
    
    width: Math.min(parent.width - Kirigami.Units.largeSpacing * 4, Kirigami.Units.gridUnit * 30)
    height: Math.min(parent.height - Kirigami.Units.largeSpacing * 4, Kirigami.Units.gridUnit * 30)
        
    ListView {
        id: feedListView
        model: EditableFeedListModel {
            sourceModel: root.feedListModel
        }
        delegate: CheckDelegate {
            required property var feed
            required property int index

            text: feed.name

            // Check if feed's flags include the UseReadableContentFlag
            checked: feed.flags & Feed.UseReadableContentFlag

            onToggled: {
                // Toggle the flag using bitwise operations
                if (checked) {
                    feed.flags |= Feed.UseReadableContentFlag
                } else {
                    feed.flags &= ~Feed.UseReadableContentFlag
                }
            }

            width: ListView.view.width
        }
    }

    footer: Control {
        padding: Kirigami.Units.smallSpacing

        contentItem: RowLayout {
            spacing: Kirigami.Units.smallSpacing

            Label {
                Layout.alignment: Qt.AlignRight
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignRight
                text: qsTr("Download articles:")
            }

            ComboBox {
                id: prefetchContent
                Layout.alignment: Qt.AlignRight
                implicitContentWidthPolicy: ComboBox.WidestTextWhenCompleted
                model: [qsTr("When opened"), qsTr("With feed updates")]
                currentIndex: globalSettings.prefetchContent ? 1 : 0
                Binding {
                    target: globalSettings
                    property: "prefetchContent"
                    value: prefetchContent.currentIndex === 1
                }
            }
        }
    }
}
