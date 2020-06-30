import QtQuick 2.12
import QtQuick.Layouts 1.12
import Qt.labs.settings 1.1
import org.kde.kirigami 2.7 as Kirigami

Kirigami.ScrollablePage {
    id: root
    topPadding: 0
    bottomPadding: 0
    leftPadding: 0
    rightPadding: 0
    title: qsTr("All Items")

    Settings {
        id: settings
        category: "ArticleList"
        property alias unreadFilter: unreadFilterAction.checked
    }

    property var pageRow: null
    property var feedFilter: null

    /* HACK: incrementing this reloads the model */
    property int modelIteration: 0

    Kirigami.Theme.colorSet: Kirigami.Theme.View
    Kirigami.Theme.backgroundColor: Kirigami.Theme.alternateBackgroundColor

    function openChild() {
        if (!pageRow) return
        while (pageRow.depth > Kirigami.ColumnView.index + 1)
            pageRow.pop()
        if (articleList.currentItem) {
            var data = articleList.currentItem.data
            root.pageRow.push("qrc:/qml/ArticlePage.qml", {item: data})
            if (data.isUnread) feedManager.setRead(data.id, true)
        }
    }

    ArticleList {
       id: articleList
       anchors.fill: parent
       onCurrentItemChanged: openChild()
       model: {
           modelIteration;
           return feedManager.getModel(feedFilter, settings.unreadFilter);
       }
   } /* ArticleList */

    actions {
        main: Kirigami.Action {
            text: qsTr("Mark All Read")
            iconName: "mail-read"
            onTriggered: {
                feedManager.setAllUnread(feedFilter, false);
                modelIteration++;
            }
            displayHint: Kirigami.Action.DisplayHint.KeepVisible
        }
         left: Kirigami.Action {
             id: unreadFilterAction
             text: qsTr("Hide Read")
             iconName: "view-filter"
             checkable: true
             checked: true
             displayHint: Kirigami.Action.DisplayHint.KeepVisible
         }
         right: Kirigami.Action {
             text: qsTr("Search")
             iconName: "search"
             checkable: true
             checked: false
             displayHint: Kirigami.Action.DisplayHint.KeepVisible | Kirigami.Action.DisplayHint.IconOnly
         }
    }
}
