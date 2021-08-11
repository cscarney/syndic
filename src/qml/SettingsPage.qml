/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick 2.0
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.0
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
    }
}
