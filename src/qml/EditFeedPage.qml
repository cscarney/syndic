import QtQuick 2.0
import org.kde.kirigami 2.7 as Kirigami
import Feed 1.0

AbstractFeedEditorPage {
    property Feed targetFeed;
    provisionalFeed {
        targetFeed: targetFeed
    }

    actions {
        main: Kirigami.Action {
            text: qsTr("Save")
            iconName: "document-save"
            onTriggered: provisionalFeed.save();
        }

        left: Kirigami.Action {
            text: qsTr("Delete")
            iconName: "delete"
            onTriggered: targetFeed.requestDelete();
        }
    }
}
