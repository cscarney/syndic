/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import org.kde.kirigami 2.7 as Kirigami
import com.rocksandpaper.syndic 1.0

Kirigami.ScrollablePage {
    id: root
    property Item pageRow;
    property ProvisionalFeed provisionalFeed
    property bool previewOpen: false

    ButtonGroup {
        id: expireAgeGroup
        exclusive: true
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
                Kirigami.FormData.label: qsTr("Feed Url:")
                text: provisionalFeed.urlString
                onTextEdited: {
                    provisionalFeed.urlString = text;
                    previewOpen = false;
                }
                onEditingFinished: provisionalFeed.syncUrlString();
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

            ElasticComboBox {
                id: categoryField
                Kirigami.FormData.label: qsTr("Category:")
                model: feedContext.getCategories()
                editable: true
                onEditTextChanged: {
                    provisionalFeed.category = editText
                }
                Component.onCompleted: {
                    currentIndex = indexOfValue(provisionalFeed.category);
                }
            }

            CheckBox {
                text: qsTr("Always load web content")
                checked: provisionalFeed.flags & Feed.UseReadableContentFlag
                onToggled: {
                    if (checked) {
                        provisionalFeed.flags |= Feed.UseReadableContentFlag
                    } else {
                        provisionalFeed.flags &= ~Feed.UseReadableContentFlag
                    }
                }
            }

            Kirigami.Separator{}

            RadioButton {
                id: updateIntervalGlobal
                ButtonGroup.group: updateIntervalGroup
                Kirigami.FormData.label: qsTr("Fetch updates:")
                //: take the value from application settings
                text: qsTr("Use Global Setting")
                checked: provisionalFeed.updateMode === Feed.InheritUpdateMode
                onToggled: {
                    if (checked) {
                        provisionalFeed.updateMode = Feed.InheritUpdateMode
                    }
                }
            }

            RadioButton {
                id: updateIntervalCustom
                ButtonGroup.group: updateIntervalGroup
                checked: provisionalFeed.updateMode === Feed.OverrideUpdateMode
                text: contentItem.displayText
                contentItem: SpinBox {
                    id: updateIntervalValue
                    KeyNavigation.backtab: updateIntervalCustom
                    anchors {
                        left: parent.indicator.right
                        verticalCenter: parent.indicator.verticalCenter
                    }
                    enabled: updateIntervalCustom.checked
                    value: provisionalFeed.updateInterval / 60
                    from: 1
                    textFromValue: (value, locale)=>qsTr("%n minute(s)", "", value)
                    valueFromText: (text, locale)=>+text.replace(/[^\d]/g, "")
                    onValueModified: provisionalFeed.updateInterval = value * 60
                }
                onToggled: {
                    if (checked) {
                        provisionalFeed.updateMode = Feed.OverrideUpdateMode
                    }
                }
            }

            RadioButton {
                id: updateIntervalDisable
                ButtonGroup.group: updateIntervalGroup
                text: qsTr("Never")
                checked: provisionalFeed.updateMode === Feed.DisableUpdateMode
                onToggled: {
                    if (checked) {
                        provisionalFeed.updateMode = Feed.DisableUpdateMode
                    }
                }
            }

            Kirigami.Separator {}

            RadioButton {
                id: expireAgeGlobal
                ButtonGroup.group: expireAgeGroup
                Kirigami.FormData.label: qsTr("Delete old items:")
                text: qsTr("Use Global Setting")
                checked: provisionalFeed.expireMode === Feed.InheritUpdateMode
                onToggled: {
                    if (checked) {
                        provisionalFeed.expireMode = Feed.InheritUpdateMode
                    }
                }
            }

            RadioButton {
                id: expireAgeCustom
                ButtonGroup.group: expireAgeGroup
                checked: provisionalFeed.expireMode === Feed.OverrideUpdateMode
                text: contentItem.displayText
                contentItem: SpinBox {
                    id: expireAgeValue
                    anchors {
                        left: parent.indicator.right
                        verticalCenter: parent.indicator.verticalCenter
                    }
                    KeyNavigation.backtab: expireAgeCustom
                    enabled: expireAgeCustom.checked
                    value: provisionalFeed.expireAge / 86400
                    from: 1
                    textFromValue: (value, locale)=>qsTr("%n day(s)", "", value)
                    valueFromText: (text, locale)=>+text.replace(/[^\d]/g, "")
                    onValueModified: provisionalFeed.expireAge = value * 86400
                }
                onToggled: {
                    if (checked) {
                        provisionalFeed.expireMode = Feed.OverrideUpdateMode
                    }
                }
            }


            RadioButton {
                id: expireAgeDisable
                ButtonGroup.group: expireAgeGroup
                text: qsTr("Never")
                checked: provisionalFeed.expireMode === Feed.DisableUpdateMode
                onToggled: {
                    if (checked) {
                        provisionalFeed.expireMode = Feed.DisableUpdateMode
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
                             provisionalFeed: provisionalFeed});
            previewOpen = true;
        } else {
            pageRow.pop(this);
        }
    }

    Component.onCompleted: urlField.focus = true
}
