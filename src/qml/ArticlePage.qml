import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import org.kde.kirigami 2.7 as Kirigami

Kirigami.ScrollablePage {
    id: root
    property alias item: articleView.item
    property var nextItem: function() {}
    property var previousItem: function() {}
    signal suspendAnimations;

    topPadding: 0
    bottomPadding: 0
    leftPadding: 0
    rightPadding: 0
    Kirigami.Theme.colorSet: Kirigami.Theme.View
    Layout.fillWidth: true
    title:  item.article.title || ""
    titleDelegate: Kirigami.Heading {
        visible: !scroller.atYBeginning
        elide: Text.ElideRight
         text: root.title
         maximumLineCount: 1
         Layout.maximumWidth: 0.66 * root.width
    }

    actions {
        main: Kirigami.Action {
            text: qsTr("Keep Unread")
            iconName: "mail-mark-unread"
            checkable: true
            checked: false
            onCheckedChanged: item.article.isRead = !checked
            displayHint: Kirigami.Action.DisplayHint.KeepVisible
        }
    }

    /* extract <img> tags from the text so that we can use them as cover images */
    QtObject {
        id: priv
        property var firstImage: ""
        property var textWithoutImages: ""
    }

    Connections {
        target: item.article
        function onGotContent(content) {
            var src = content || ""
            var images = []
            var text = src.replace(/<img .*src="([^"]*)".*>/ig, function(match, src){
                images.push(src)
                return "";
            })
            priv.firstImage = images[0] || ""
            priv.textWithoutImages = text
        }
    }

    /* Gets reparented to the overlay by ScrollablePage */
    OverlayMessage {
        id: hoveredLinkToolTip
        text: articleView.hoveredLink
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
    }

    Flickable {
        id: scroller
        anchors.fill: parent

        /* setting the content width based on the page width, rather
          than the flickable width to avoid a binding loop:
          content height -> scrollbar visibility -> content width -> content height */
        contentWidth: root.width - leftMargin - rightMargin

        contentHeight: articleView.height + bottomMargin + topMargin
        clip: true
        flickableDirection: Flickable.AutoFlickIfNeeded
        topMargin: 20
        bottomMargin: 20
        leftMargin: 35
        rightMargin: 35
        ArticleView {
            id: articleView
            width: scroller.contentWidth
            firstImage: priv.firstImage
            textWithoutImages: priv.textWithoutImages
        }

        Behavior on contentY {
            id: animateScroll
            enabled: false
            NumberAnimation {
                duration: Kirigami.Units.longDuration
                easing.type: Easing.OutExpo
                onRunningChanged: if (!running) scroller.returnToBounds()
            }
        }
    }

    Keys.onPressed: {
        switch (event.key) {
        case Qt.Key_Space:
            if (!scroller.atYEnd) {
                pageUpDown(0.9);
            } else {
                suspendAnimations();
                nextItem();
            }
            break;

        case Qt.Key_Left:
            suspendAnimations();
            previousItem();
            break;

        case Qt.Key_Right:
            suspendAnimations();
            nextItem();
            break;

        case Qt.Key_PageUp:
            pageUpDown(-0.9);
            break;

        case Qt.Key_PageDown:
            pageUpDown(0.9);
            break;

        case Qt.Key_Return:
        case Qt.Key_Enter:
            Qt.openUrlExternally(item.article.url);
            break;

        default:
            return;
        }
        event.accepted = true
    }

    Component.onCompleted: {
        item.article.requestContent();
    }

    function pageUpDown(increment) {
        animateScroll.enabled = true
        scroller.contentY = scroller.contentY + (increment * scroller.height)
        animateScroll.enabled = false
    }
}
