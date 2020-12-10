import FeedItemModel 1.0

AbstractItemListPage {
    id: root
    property var feed

    model: FeedItemModel {
        id: feedItemModel
        manager: feedManager
        feed: root.feed
        unreadFilter: root.unreadFilter
    }
}
