import QtQuick 2.0
import org.kde.kirigami 2.7 as Kirigami

AbstractFeedEditorPage {
    title: qsTr("Add Content")

    actions {
        main: Kirigami.Action {
            id: previewAction
            text: qsTr("Preview...")
            iconName: "document-preview"
            checkable: true
            checked: previewOpen
            onToggled: previewOpen = checked
        }
    }

    Keys.onReturnPressed: previewOpen = true
    Keys.onEnterPressed: previewOpen = true
}
