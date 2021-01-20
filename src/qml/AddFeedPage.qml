import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import org.kde.kirigami 2.7 as Kirigami
import ProvisionalFeed 1.0
import Updater 1.0

Kirigami.ScrollablePage {
    id: root
    property Item pageRow;
    property alias previewOpen: previewAction.checked

    title: qsTr("Add Content")

    actions {
        main: Kirigami.Action {
            id: previewAction
            text: qsTr("Preview...")
            iconName: "document-preview"
            checkable: true
            checked: false
        }
    }

    ProvisionalFeed {
        id: provisionalFeed
        url: urlField.text
        updater {
            updateMode: updateIntervalGroup.updateMode
            updateInterval: updateIntervalValue.value * 60
        }
        onNameChanged: nameField.text = name;
    }

    ButtonGroup {
        id: updateIntervalGroup
        property int updateMode: checkedButton.updateMode
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
                    onTextChanged: previewOpen = false;
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
                    onTextChanged: provisionalFeed.name = text
                }

                RadioButton {
                    id: updateIntervalGlobal
                    property int updateMode: Updater.DefaultUpdateMode
                    ButtonGroup.group: updateIntervalGroup
                    Kirigami.FormData.label: qsTr("Update Interval:")
                    text: qsTr("Use Global Setting")
                    checked: true
                }

                RadioButton {
                    id: updateIntervalDisable
                    property int updateMode: Updater.MaunualUpdateMode
                    ButtonGroup.group: updateIntervalGroup
                    text: qsTr("Disable Automatic Updates")
                }

                RadioButton {
                    id: updateIntervalCustom
                    property int updateMode: Updater.CustomUpdateMode
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

    onPreviewOpenChanged: {
        if (previewOpen) {
            openPreview();
        } else {
            pageRow.pop(this);
        }
    }

    Keys.onReturnPressed: openPreview();
    Keys.onEnterPressed: openPreview();

    Component.onCompleted: urlField.focus = true

    function openPreview() {
        provisionalFeed.updater.start()
        pageRow.currentIndex = Kirigami.ColumnView.index
        pageRow.push("qrc:/qml/ArticleList/FeedPreviewPage.qml",
                     {pageRow: pageRow,
                         feed: provisionalFeed});
        previewOpen = true;
    }
}
