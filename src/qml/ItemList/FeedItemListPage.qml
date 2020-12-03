import FeedItemModel 1.0

AbstractItemListPage {
    id: root
    property var feedId: 0;

    model: FeedItemModel {
        manager: feedManager
        feedId: root.feedId
        unreadFilter: root.unreadFilter
    }
}
