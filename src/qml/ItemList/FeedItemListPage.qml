import FeedItemModel 1.0

AbstractItemListPage {
    id: root
    property var feedRef
    property var feed: feedRef.feed
    title: feed.name

    model: FeedItemModel {
        id: feedItemModel
        manager: feedManager
        feed: root.feedRef
        unreadFilter: root.unreadFilter
    }
}
