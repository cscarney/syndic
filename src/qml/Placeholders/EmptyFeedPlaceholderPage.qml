import QtQuick 2.0
import org.kde.kirigami 2.7 as Kirigami

AbstractPlaceholderPage {
    property var model;

    iconName: "mail-read-symbolic"
    headingText: "All Read"

    actions {
        main: Kirigami.Action {
            text: qsTr("Refresh Feed")
            iconName: "view-refresh"
            onTriggered: model.requestUpdate()
        }
    }
}
