import QtQuick 2.0
import org.kde.kirigami 2.7 as Kirigami

AbstractPlaceholderPage {
    property var model;

    iconName: "Finished"
    headingText: "All Read"

    actions {
        main: Kirigami.Action {
            text: qsTr("Refresh Feed")
            iconName: "reload"
            onTriggered: model.requestUpdate()
        }
    }
}
