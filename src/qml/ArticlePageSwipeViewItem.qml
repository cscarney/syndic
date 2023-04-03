import QtQuick 2.12
import QtQuick.Controls 2.15
import org.kde.kirigami 2.14 as Kirigami
import com.rocksandpaper.syndic 1.0

ScrollView {
    id: scrollView
    property alias showExpandedByline: articleView.showExpandedByline
    property alias hoveredLink: articleView.hoveredLink
    property alias item: articleView.item
    property bool isReadable: false

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
                console.log("set content to " + content)
            }
        }
    }

    Component.onCompleted: {
        requestContent();
        root.isReadableChanged.connect(requestContent)
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
