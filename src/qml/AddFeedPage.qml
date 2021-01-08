import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import org.kde.kirigami 2.7 as Kirigami
import ProvisionalFeed 1.0

Kirigami.ScrollablePage {
    id: root
    property Item pageRow;

    title: qsTr("Add Content")

    actions {
        main: Kirigami.Action {
            id: saveAction
            text: qsTr("Save")
            iconName: "dialog-ok"
            onTriggered: {
                var feed = provisionalFeed
                pageRow.clear()
                feedContext.addFeed(provisionalFeed.url)
            }
        }
    }

    ProvisionalFeed {
        id: provisionalFeed
        url: urlField.text
    }

    ButtonGroup {
        id: updateIntervalGroup
        exclusive: true
    }

    Flickable {
        contentWidth: Math.max(width, mainForm.implicitWidth)
        contentHeight: mainForm.implicitHeight
        ColumnLayout {
            id: mainForm
            anchors {
                top: parent.top
                bottom: parent.bottom
                horizontalCenter: parent.horizontalCenter
            }

            Kirigami.FormLayout {
                id: optionsForm
                TextField {
                    id: urlField
                    Kirigami.FormData.label: qsTr("Feed Url:")
                }

                Item {
                    Layout.preferredWidth: urlField.width
                    Layout.minimumHeight:previewButton.height
                    Button {
                        id: previewButton
                        text: qsTr("Preview...")
                        anchors.right: parent.right
                        onClicked: openPreview();
                    }
                }

                Item {
                    Kirigami.FormData.isSection: true
                    Kirigami.FormData.label: qsTr("Feed Options")
                }

                TextField {
                    id: nameField
                    Kirigami.FormData.label: qsTr("Display Name:")
                }

                RadioButton {
                    id: updateIntervalGlobal
                    ButtonGroup.group: updateIntervalGroup
                    Kirigami.FormData.label: qsTr("Update Interval:")
                    text: qsTr("Use Global Setting")
                }

                RadioButton {
                    id: updateIntervalDisable
                    ButtonGroup.group: updateIntervalGroup
                    text: qsTr("Disable Automatic Updates")
                }

                RadioButton {
                    id: updateIntervalCustom
                    ButtonGroup.group: updateIntervalGroup
                    contentItem: SpinBox {
                        id: updateIntervalValue
                        anchors {
                            left: parent.indicator.right
                            verticalCenter: parent.indicator.verticalCenter
                        }
                        KeyNavigation.backtab: updateIntervalCustom
                        enabled: parent.checked
                        textFromValue: function(value, locale) {
                            return qsTr("%n minute(s)", "", value)
                        }
                    }
                }
            }
        }
    }

    Component.onCompleted: urlField.focus = true

    function openPreview() {
        provisionalFeed.updater.start()
        pageRow.push("qrc:/qml/ArticleList/ArticleListPage.qml",
                     {pageRow: pageRow,
                         feed: provisionalFeed});
    }
}
