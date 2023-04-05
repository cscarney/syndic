import QtQuick 2.12
import QtQuick.Controls 2.15
import org.kde.kirigami 2.14 as Kirigami
import com.rocksandpaper.syndic 1.0

ScrollView {
    id: root
    required property var item
    property alias showExpandedByline: articleView.showExpandedByline
    property alias hoveredLink: articleView.hoveredLink
    property bool isReadable: item.article ? item.article.feed.flags & Feed.UseReadableContentFlag : false
    property bool inProgress: false

    clip: true
    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

    Flickable {
        id: scroller
        anchors.fill: parent
        Keys.forwardTo: [root]

        contentWidth: root.width - leftMargin - rightMargin
        contentHeight: articleView.height + bottomMargin + topMargin
        clip: true
        flickableDirection: Flickable.AutoFlickIfNeeded
        topMargin: Kirigami.Units.gridUnit
        bottomMargin: Kirigami.Units.gridUnit
        leftMargin: Kirigami.Units.gridUnit * 1.6
        rightMargin: Kirigami.Units.gridUnit * 1.6
        ArticleView {
            id: articleView
            item: root.item
            width: scroller.contentWidth
        }

        Behavior on contentY {
            id: animateScroll
            NumberAnimation {
                duration: Kirigami.Units.shortDuration
                easing.type: Easing.OutExpo
            }
        }
    }

    Connections {
        target: item.article
        function onGotContent(content, type) {
            if (root.isReadable || (type===Article.FeedContent)) {
                root.inProgress = false;
                articleView.text = content;
            }
        }
    }

    Component.onCompleted: {
        requestContent();
        root.isReadableChanged.connect(requestContent)

        // The margins can get squeezed out of the viewport while the
        // size bindings are being evaluated during delegate creation,
        // fix that here
        scroller.returnToBounds()
    }

    function requestContent(forceReload) {
        root.inProgress = true;
        if (root.isReadable) {
            item.article.requestReadableContent(feedContext, !!forceReload);
        } else {
            item.article.requestContent();
        }
    }

    function refreshReadable() {
        articleView.text = "";
        requestContent(true);
    }

    function pxUpDown(increment) {
        const topY = scroller.originY - scroller.topMargin;
        const bottomY = scroller.originY + scroller.contentHeight + scroller.bottomMargin - scroller.height;
        scroller.contentY = Math.max(topY, Math.min(scroller.contentY + increment, bottomY))
    }

    function pageUpDown(increment) {
        pxUpDown(increment * scroller.height)
    }
}
