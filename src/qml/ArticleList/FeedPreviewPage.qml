import Enums 1.0
import org.kde.kirigami 2.7 as Kirigami

AbstractArticleListPage {
    id: root
    automaticOpen: false

    actions {
        main: Kirigami.Action {
            id: saveAction
            text: qsTr("Subscribe")
            iconName: "list-add"
            enabled: model.status == Enums.Idle
            onTriggered: {
                var feed = root.feed
                pageRow.clear();
                feedContext.addFeed(feed);
            }
        }
    }
}
