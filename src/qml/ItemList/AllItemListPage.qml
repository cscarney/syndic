import QtQuick 2.0
import AllItemModel 1.0
import FeedManager 1.0

AbstractItemListPage {
    id: root

    model: AllItemModel {
        manager: feedManager
        unreadFilter: root.unreadFilter
    }
}
