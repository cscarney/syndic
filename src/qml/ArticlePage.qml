import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import org.kde.kirigami 2.7 as Kirigami

Kirigami.ScrollablePage {
    id: articlePage
    property alias item: articleView.item
    topPadding: 0
    bottomPadding: 0
    leftPadding: 0
    rightPadding: 0
    Kirigami.Theme.colorSet: Kirigami.Theme.View
    Layout.fillWidth: true
    title:  item.headline || ""
    // @disable-check M16
    titleDelegate: Kirigami.Heading {
        visible: !scroller.atYBeginning
        elide: Text.ElideRight
         text: articlePage.title
         maximumLineCount: 1
         Layout.maximumWidth: 0.66 * articlePage.width
    }

    /* extract <img> tags from the text so that we can use them as cover images */
    property var splitContent: {
        var src = item ? item.content || "" : ""
        var images = []
        var text = src.replace(/<img .*src="([^"]*)".*>/ig, function(match, src){
            images.push(src)
            return "";
        })
        return {images: images, text:text}
    }

    Flickable {
        id: scroller
        anchors.fill: parent

        /* setting the content width based on the page width, rather
          than the flickable width to avoid a binding loop:
          content height -> scrollbar visibility -> content width -> content height */
        contentWidth: articlePage.width - leftMargin - rightMargin

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
            firstImage: splitContent.images[0] || ""
            textWithoutImages: splitContent.text
        }
    }

    actions {
        main: Kirigami.Action {
            text: qsTr("Keep Unread")
            iconName: "mail-mark-unread"
            checkable: true
            checked: false
            onCheckedChanged: feedManager.setRead(item.id, !checked)
            displayHint: Kirigami.Action.DisplayHint.KeepVisible
        }
        left: Kirigami.Action {
            text: qsTr("Star")
            iconName: checked ? "starred-symbolic" : "non-starred-symbolic"
            checkable: true
            checked: item.isStarred ? true : false
            onCheckedChanged: feedManager.setStarred(item.id, checked)
            displayHint: Kirigami.Action.DisplayHint.KeepVisible
        }
        /* right: Kirigami.Action {
            text: qsTr("Share...")
            iconName: "document-share"
        } */
    }
}
