import QtQuick
import QtQuick.Controls
import org.kde.kirigami as Kirigami
import com.rocksandpaper.syndic

AbstractArticleListPage {
    id: root
    required property Feed feed
    property bool acceptFeedListAction: false

    model: FeedModel {
        id: feedItemModel
        feed: root.feed
        unreadFilter: root.unreadFilter
    }

    delegate: ItemDelegate {
        id: articleListItem
        required property var ref
        required property int index // needed by Kirigami
        width: ListView.view?.width ?? implicitWidth
        text: ref.article.headline
        padding: 10
        horizontalPadding: padding * 2
        opacity: enabled ? 1 : 0.6
        highlighted: ListView.isCurrentItem
        hoverEnabled: !Kirigami.Settings.hasTransientTouchInput

        contentItem: ArticleListEntry {
            article: ref.article
        }
        onClicked: {
            root.selectIndex(index)
        }
    } /*  delegate */

    function feedListAction() {
        // ignore the one that created us...
        if (!acceptFeedListAction) return;
        clearRead();
    }

    Component.onCompleted: Qt.callLater(function(){
        acceptFeedListAction = true;
    })
}
