import QtQuick 2.12
import com.rocksandpaper.syndic

AbstractArticleListPage {
    required property Feed feed

    model: FeedModel {
        id: feedItemModel
        feed: root.feed
        unreadFilter: root.unreadFilter
    }
}
