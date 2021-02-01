import org.kde.kirigami 2.7 as Kirigami

AbstractArticleListPage {
    id: root

    actions {
        main: Kirigami.Action {
            text: qsTr("Mark All Read")
            iconName: "mail-read"
            enabled: root.count > 0
            onTriggered: {
                root.model.markAllRead();
            }
            displayHint: Kirigami.Action.DisplayHint.KeepVisible
        }
         left: Kirigami.Action {
             text: qsTr("Unread Only")
             iconName: "view-filter"
             checkable: true
             checked: root.unreadFilter
             displayHint: Kirigami.Action.DisplayHint.KeepVisible
             onCheckedChanged: root.unreadFilter = checked
         }
        contextualActions: [
            Kirigami.Action {
                text: qsTr("Edit...")
                iconName: "document-edit"
                displayHint: Kirigami.Action.AlwaysHide
                visible: feed.editable
                onTriggered: {
                    pageRow.pop(root)
                    pageRow.push("qrc:/qml/EditFeedPage.qml", {targetFeed: feed});
                }
            }

        ]
    }
}
