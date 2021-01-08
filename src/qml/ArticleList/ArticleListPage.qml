import QtQuick 2.12
import QtQuick.Layouts 1.12
import Qt.labs.settings 1.1
import org.kde.kirigami 2.7 as Kirigami
import Enums 1.0
import ArticleListModel 1.0
import Feed 1.0

Kirigami.ScrollablePage {
    id: root

    property var feed
    property var pageRow: null
    property alias model: articleList.model;
    property alias unreadFilter: settings.unreadFilter
    signal suspendAnimations

    topPadding: 0
    bottomPadding: 0
    leftPadding: 0
    rightPadding: 0 
    title: feed.name
    Kirigami.Theme.colorSet: Kirigami.Theme.View
    Kirigami.Theme.backgroundColor: Kirigami.Theme.alternateBackgroundColor

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
             text: qsTr("Unread Only")
             iconName: "view-filter"
             checkable: true
             checked: true
             displayHint: Kirigami.Action.DisplayHint.KeepVisible
         }
    }

    EmptyFeedOverlay {
        id: emptyOverlay
        anchors.fill: parent
        visible: articleList.count == 0
    }

    ArticleListView {
       id: articleList
       anchors.fill: parent
       currentIndex: -1
       onCurrentItemChanged: openChild()

       model: ArticleListModel {
          id: feedItemModel
          feed: root.feed
          unreadFilter: root.unreadFilter
          onStatusChanged: {
              if (articleList.currentItem) return;
              if ((model.status === Enums.Idle) && (model.rowCount() > 0))
                  articleList.currentIndex = 0;
              else openChild();
          }
      }
   } /* ArticleList */

    Settings {
        id: settings
        // @disable-check M16
        category: "ArticleList"
        property alias unreadFilter: unreadFilterAction.checked
    }

    function pushPlaceholder() {
        switch(model.status) {
        case Enums.Loading:
            // just wait for the load...
            break;
        case Enums.Updating:
            root.pageRow.push("qrc:/qml/Placeholders/UpdatingPlaceholderPage.qml");
            break;
        case Enums.Error:
            root.pageRow.push("qrc:/qml/Placeholders/ErrorPlaceholderPage.qml", {model:model})
            break;
        default:
            root.pageRow.push("qrc:/qml/Placeholders/EmptyFeedPlaceholderPage.qml", {model:model});
        }
    }

    function openChild() {
        if (!pageRow) return;
        pageRow.currentIndex = Kirigami.ColumnView.index
        if (pageRow.wideMode) {
            suspendAnimations();
        }
        if (articleList.currentItem) {
            var data = articleList.currentItem.data
            root.pageRow.push("qrc:/qml/ArticlePage.qml", {item: data, nextItem: nextItem, previousItem: previousItem})
            data.article.isRead = true
        } else if (model) {
            pushPlaceholder();
        }
    }

    function nextItem () { articleList.currentIndex++ }
    function previousItem () { articleList.currentIndex-- }
}
