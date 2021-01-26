import QtQuick 2.0
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.0
import org.kde.kirigami 2.7 as Kirigami

Kirigami.ScrollablePage {
    id: root
    property var globalSettings;

    title: qsTr("Settings")

    Flickable {
        contentWidth: Math.max(width, mainForm.implicitWidth)
        contentHeight: mainForm.implicitHeight
        Kirigami.FormLayout {
            id: mainForm

            anchors {
                top: parent.top
                bottom: parent.bottom
                horizontalCenter: parent.horizontalCenter
            }

            CheckBox {
                id: updateOnStart
                Kirigami.FormData.label: qsTr("Fetch updates:")
                enabled: false
                checked: false
                text: qsTr("On application start")
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
                    textFromValue: function(value, locale) {
                        return qsTr("%n minutes", "", value)
                    }
                    Binding {
                        target: globalSettings
                        property: "updateInterval"
                        value: updateIntervalValue.value * 60
                    }
                }
            }
        }
    }
}
