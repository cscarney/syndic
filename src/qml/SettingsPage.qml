/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick 2.0
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.0
import QtQuick.Dialogs
import org.kde.kirigami 2.7 as Kirigami
import com.rocksandpaper.syndic 1.0

Kirigami.ScrollablePage {
    id: root
    property bool keepDrawerOpen: true
    required property FeedListModel feedListModel

    title: qsTr("Settings")

    Kirigami.FormLayout {
        id: mainForm

        CheckBox {
            id: updateOnStart
            Kirigami.FormData.label: qsTr("Fetch updates:")
            checked: globalSettings.updateOnStart
            text: qsTr("On application start")
            Binding {
                target: globalSettings
                property: "updateOnStart"
                value: updateOnStart.checked
            }
        }

        RowLayout {
            CheckBox {
                id: updateIntervalEnabled
                checked: globalSettings.automaticUpdates
                text: qsTr("Automatically every:")
                Binding {
                    target: globalSettings
                    property: "automaticUpdates"
                    value: updateIntervalEnabled.checked
                }
            }
            SpinBox {
                id: updateIntervalValue
                enabled: updateIntervalEnabled.checked
                value: globalSettings.updateInterval / 60
                from: 1
                textFromValue: (value, locale)=>qsTr("%n minute(s)", "", value)
                valueFromText: (text, locale)=>+text.replace(/[^\d]/g, "")
                Binding {
                    target: globalSettings
                    property: "updateInterval"
                    value: updateIntervalValue.value * 60
                }
            }
        }

        CheckBox {
            id: runInBackground
            text: qsTr("Run in background")
            visible: !Kirigami.Settings.isMobile
            checked: globalSettings.runInBackground
            Binding {
                target: globalSettings
                property: "runInBackground"
                value: runInBackground.checked
            }
        }

        RowLayout {
            Kirigami.FormData.label: qsTr("Delete old items:")
            CheckBox {
                id: expireItems
                checked: globalSettings.expireItems
                Binding {
                    target: globalSettings
                    property: "expireItems"
                    value: expireItems.checked
                }
            }
            SpinBox {
                id: expireAge
                enabled: expireItems.checked
                value: globalSettings.expireAge / 86400
                from: 1
                textFromValue: (value, locale)=>qsTr("%n day(s)", "", value)
                valueFromText: (text, locale)=>+text.replace(/[^\d]/g, "")
                Binding {
                    target: globalSettings
                    property: "expireAge"
                    value: expireAge.value * 86400
                }
            }
        }

        ComboBox {
            id: startPage
            Kirigami.FormData.label: qsTr("Start page:");
            implicitContentWidthPolicy: ComboBox.WidestTextWhenCompleted
            model: [qsTr("Highlights"), qsTr("All Items")];
            currentIndex:  globalSettings.startPage
            Binding {
                target: globalSettings
                property: "startPage"
                value: startPage.currentIndex
            }
        }

        ComboBox {
            id: feedListSort
            Kirigami.FormData.label: qsTr("Sort feed list:");
            implicitContentWidthPolicy: ComboBox.WidestTextWhenCompleted
            model: [qsTr("Alphabetical"), qsTr("Unread First")]            
            currentIndex: globalSettings.feedListSort
            Binding {
                target: globalSettings
                property: "feedListSort"
                value: feedListSort.currentIndex
            }
        }

        ComboBox {
            id: articleFont
            Kirigami.FormData.label: qsTr("Article font:")
            implicitContentWidthPolicy: ComboBox.WidestTextWhenCompleted
            model: [qsTr("Use system font", "entry in font list"), "serif", ...Qt.fontFamilies()]
            onActivated: {
                if (currentIndex === 0) {
                    globalSettings.bodyFont = "";
                } else {
                    globalSettings.bodyFont = currentText
                }
            }
            Component.onCompleted: {
                if (!globalSettings.bodyFont) {
                    currentIndex = 0;
                } else {
                    currentIndex = find(globalSettings.bodyFont);
                }
            }
        }
        
        Button {
            Kirigami.FormData.label: qsTr("Web content:")
            text: qsTr("Configure…")
            onClicked: {
                dialogLoader.sourceComponent = readableContentDialogComponent;
                const dialog = dialogLoader.item;
                dialog.open();
            }
        }

        RowLayout {
            Kirigami.FormData.label: qsTr("OPML Data:")
            Button {
                text: qsTr("Import…");
                onClicked: {
                    dialogLoader.sourceComponent = dialogComponent;
                    const opmlDialog = dialogLoader.item;
                    opmlDialog.fileMode = FileDialog.SaveFile;
                    opmlDialog.acceptedFunc = function() {
                        feedContext.importOpml(opmlDialog.selectedFile);
                    }
                    opmlDialog.open();
                }
            }

            Button {
                text: qsTr("Export…")
                onClicked: {
                    dialogLoader.sourceComponent = dialogComponent;
                    const opmlDialog = dialogLoader.item;
                    opmlDialog.fileMode = FileDialog.OpenFile;
                    opmlDialog.acceptedFunc = function() {
                        feedContext.exportOpml(opmlDialog.selectedFile);
                    }
                    opmlDialog.open();
                }
            }
        }
    }

    resources: [
        Loader {
            id: dialogLoader
        },

        Component {
            id: dialogComponent;

            FileDialog {
                property var acceptedFunc: function(){}
                onAccepted: acceptedFunc();
            }
        },
        
        Component {
            id: readableContentDialogComponent;
            
            ReadableContentSettingsDialog {
                feedListModel: root.feedListModel
            }
        }
    ]
}
