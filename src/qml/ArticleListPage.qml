import QtQuick 2.12
import QtQuick.Layouts 1.12
import Qt.labs.settings 1.1
import org.kde.kirigami 2.7 as Kirigami
import ItemModel 1.0
import AllItemModel 1.0
import FeedManager 1.0

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
    property alias model: articleList.model;
    property alias unreadFilter: unreadFilterAction.checked

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
        } else if (model) {
            root.pageRow.push("qrc:/qml/NoContentPage.qml", {model: model});
        }
    }

    ArticleList {
       id: articleList
       anchors.fill: parent
       currentIndex: -1
       onCurrentItemChanged: openChild()

       Connections {
           target: articleList.model
           onStatusChanged: {
               console.log("status changed to "+model.status)
               if (model.status === ItemModel.Ok)
                   articleList.currentIndex = 0;
               else if (!articleList.currentItem) openChild();
           }
       }

       EmptyFeedOverlay {
           id: emptyOverlay
           anchors.fill: parent
           visible: articleList.count == 0
       }
   } /* ArticleList */

    actions {
        main: Kirigami.Action {
            text: qsTr("Mark All Read")
            iconName: "mail-read"
            enabled: articleList.count > 0
            onTriggered: {
                model.markAllRead();
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
