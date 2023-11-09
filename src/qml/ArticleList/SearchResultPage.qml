import QtQuick 2.12
import com.rocksandpaper.syndic

AbstractFeedPage {
    id: root
    required property string query

    title: qsTr("Search results for \"%1\"").arg(root.query)
    unreadFilter: false

    feed: SearchResultFeed {
        context: feedContext
        query: root.query
    }
}
