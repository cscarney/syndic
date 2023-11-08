import QtQuick
import QtQuick.Controls
import com.rocksandpaper.syndic

AbstractArticleListPage {
    id: root
    required property Feed feed

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

        contentItem: ArticleListEntry {
            article: ref.article
        }
        onClicked: {
            root.selectIndex(index)
        }
    } /*  delegate */
}
