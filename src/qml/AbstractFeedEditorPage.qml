/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import org.kde.kirigami 2.7 as Kirigami
import ProvisionalFeed 1.0
import Feed 1.0

Kirigami.ScrollablePage {
    id: root
    property Item pageRow;
    property bool previewOpen: false

    property ProvisionalFeed provisionalFeed: ProvisionalFeed {
        updateMode: Feed.DefaultUpdateMode
        updateInterval: 3600
    }

    ButtonGroup {
        id: updateIntervalGroup
        exclusive: true
    }

    ColumnLayout {
        id: mainForm

        Kirigami.FormLayout {
            id: optionsForm
            TextField {
                id: urlField
                readOnly: !!provisionalFeed.targetFeed
                Kirigami.FormData.label: qsTr("Feed Url:")
                text: provisionalFeed.targetFeed ? provisionalFeed.url : ""
                onTextChanged: previewOpen = false;
                onTextEdited: provisionalFeed.url = text
            }

            Item {
                Kirigami.FormData.isSection: true
                Kirigami.FormData.label: qsTr("Feed Options")
            }

            TextField {
                id: nameField
                Kirigami.FormData.label: qsTr("Display Name:")
                text: provisionalFeed.name
                onTextEdited: provisionalFeed.name = text
            }

            RadioButton {
                id: updateIntervalGlobal
                ButtonGroup.group: updateIntervalGroup
                Kirigami.FormData.label: qsTr("Update Interval:")
                text: qsTr("Use Global Setting")
                checked: provisionalFeed.updateMode === Feed.DefaultUpdateMode
                onToggled: {
                    if (checked) {
                        provisionalFeed.updateMode = Feed.DefaultUpdateMode
                    }
                }
            }

            RadioButton {
                id: updateIntervalDisable
                property int updateMode: Feed.ManualUpdateMode
                ButtonGroup.group: updateIntervalGroup
                text: qsTr("Disable Automatic Updates")
                checked: provisionalFeed.updateMode === Feed.ManualUpdateMode
                onToggled: {
                    if (checked) {
                        provisionalFeed.updateMode = Feed.ManualUpdateMode
                    }
                }
            }

            RadioButton {
                id: updateIntervalCustom
                ButtonGroup.group: updateIntervalGroup
                checked: provisionalFeed.updateMode === Feed.CustomUpdateMode
                contentItem: SpinBox {
                    id: updateIntervalValue
                    anchors {
                        left: parent.indicator.right
                        verticalCenter: parent.indicator.verticalCenter
                    }
                    KeyNavigation.backtab: updateIntervalCustom
                    enabled: parent.checked
                    value: provisionalFeed.updateInterval / 60
                    from: 1
                    textFromValue: function(value, locale) {
                        return qsTr("%n minute(s)", "", value)
                    }
                    valueFromText: function(text, locale) {
                        return +text.replace(/[^\d]/g, "")
                    }
                    onValueModified: provisionalFeed.updateInterval = value * 60
                }
                onToggled: {
                    if (checked) {
                        provisionalFeed.updateMode = Feed.CustomUpdateMode
                    }
                }
            }
        }
    }

    onPreviewOpenChanged: {
        if (previewOpen) {
            provisionalFeed.updater.start()
            pageRow.currentIndex = Kirigami.ColumnView.index
            pageRow.push("qrc:/qml/ArticleList/FeedPreviewPage.qml",
                         {pageRow: pageRow,
                             feed: provisionalFeed});
            previewOpen = true;
        } else {
            pageRow.pop(this);
        }
    }

    Component.onCompleted: urlField.focus = true
}
