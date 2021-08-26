/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick 2.0
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.0
import QtQuick.Dialogs 1.2
import org.kde.kirigami 2.7 as Kirigami

Kirigami.ScrollablePage {
    id: root
    property bool keepDrawerOpen: true
    property var globalSettings;

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
                textFromValue: function(value, locale) {
                    return qsTr("%n minute(s)", "", value)
                }
                valueFromText: function(text, locale) {
                    return +text.replace(/[^\d]/g, "")
                }
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
            checked: globalSettings.runInBackground
            Binding {
                target: globalSettings
                property: "runInBackground"
                value: runInBackground.checked
            }
        }

        RowLayout {
            Kirigami.FormData.label: qsTr("Delete after:")
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
                textFromValue: function(value, locale) {
                    return qsTr("%n day(s)", "", value)
                }
                valueFromText: function(text, locale) {
                    return +text.replace(/[^\d]/g, "")
                }
                Binding {
                    target: globalSettings
                    property: "expireAge"
                    value: expireAge.value * 86400
                }
            }
        }

        RowLayout {
            Kirigami.FormData.label: qsTr("OPML Data:")
            Button {
                text: qsTr("Import...");
                onClicked: {
                    opmlDialog.selectExisting = true;
                    opmlDialog.acceptedFunc = function() {
                        feedContext.importOpml(opmlDialog.fileUrl);
                    }
                    opmlDialog.open();
                }
            }

            Button {
                text: qsTr("Export...")
                onClicked: {
                    opmlDialog.selectExisting = false;
                    opmlDialog.acceptedFunc = function() {
                        feedContext.exportOpml(opmlDialog.fileUrl);
                    }
                    opmlDialog.open();
                }
            }
        }
    }

    FileDialog {
        id: opmlDialog
        property var acceptedFunc: function(){}
        onAccepted: acceptedFunc();
    }
}
