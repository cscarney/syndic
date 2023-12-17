import QtQuick 2.12
import QtQuick.Controls 2.15
import org.kde.kirigami 2.14 as Kirigami
import com.rocksandpaper.syndic 1.0

ScrollView {
    id: root
    required property Article article
    property alias showExpandedByline: articleView.showExpandedByline
    property alias hoveredLink: articleView.hoveredLink
    property bool isReadable: root.article?.feed.flags & Feed.UseReadableContentFlag
    property bool inProgress: false
    property alias atYEnd: scroller.atYEnd

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
            article: root.article
            width: scroller.contentWidth
        }
    }

    Connections {
        target: root.article
        function onGotContent(content, type) {
            if (root.isReadable || (type===Article.FeedContent)) {
                root.inProgress = false;
                articleView.text = content;
            }
        }
    }

    NumberAnimation {
        id: animateScroll
        target: scroller
        property: "contentY"
        duration: Kirigami.Units.shortDuration
        easing.type: Easing.OutExpo
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
            root.article.requestReadableContent(feedContext, !!forceReload);
        } else {
            root.article.requestContent();
        }
    }

    function animateScrollToY(y) {
        animateScroll.stop()
        animateScroll.from = scroller.contentY;
        animateScroll.to = y;
        animateScroll.start();
    }

    function refreshReadable() {
        articleView.text = "";
        requestContent(true);
    }

    function topY() {
        return scroller.originY - scroller.topMargin;
    }

    function bottomY() {
        return scroller.originY + scroller.contentHeight + scroller.bottomMargin - scroller.height;
    }

    function pxUpDown(increment) {
        animateScrollToY(Math.max(topY(), Math.min(scroller.contentY + increment, bottomY())))
    }

    function pageUpDown(increment) {
        pxUpDown(increment * scroller.height)
    }

    function scrollToTop() {
        animateScrollToY(topY());
    }

    function scrollToBottom() {
        animateScrollToY(bottomY());
    }
}
