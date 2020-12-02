import QtQuick 2.0

AbstractPlaceholderPage {
    property var model;

    iconName: "dialog-error-symbolic"
    headingText: "Error Updating Feed"
    descriptionText: "This feed is currently offline"

    actions {
        main: Kirigami.Action {
            text: qsTr("Retry")
            iconName: "reload"
            onTriggered: model.requestUpdate()
        }
    }
}
