import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import com.rocksandpaper.syndic
import org.kde.kirigami as Kirigami
import ".."

AbstractArticleListPage {
    id: root
    title: qsTr("Highlights")
    model: OverviewModel {
        context: feedContext
    }

    delegate: Kirigami.AbstractCard {
        id: itemCard
        required property var ref
        required property int index
        property Article article: ref.article
        visible: false
        highlighted: ListView.isCurrentItem
        showClickFeedback: true
        contentItem: ColumnLayout {
            RowLayout {
                FeedIcon {
                    feed: article.feed
                }

                Label {
                    text: article.feed.name
                    elide: Text.ElideRight
                }
            }

            Kirigami.Heading {
                text: article.title
                textFormat: Text.StyledText
                wrapMode: Text.Wrap
                Layout.maximumWidth: parent.width
            }

            Label {
                id: contentLabel
                wrapMode: Text.Wrap
                elide: Text.ElideRight
                textFormat: Text.PlainText
                maximumLineCount: 3
                Layout.maximumWidth: parent.width
            }
        }

        Binding {
            itemCard.background.opacity: article.isRead && !highlighted ? 0.5 : 1
        }

        Connections {
            target: article
            function onGotContent(content, type) {
                if (type !== Article.FeedContent) {
                    return;
                }

                // TODO replace this with a proper HTML parse
                contentLabel.text = content.replace(/<[^>]*>/g, "").replace(/\s+/g, " ").trim();
                itemCard.visible = true
            }
        }

        onClicked: {
            root.selectIndex(index)
        }

        Component.onCompleted: {
            article.requestContent()
        }
    }

    Binding {
        root.view.spacing: Kirigami.Units.largeSpacing * 2
        root.view.topMargin: root.view.spacing
        root.view.bottomMargin: 1000
        root.view.rightMargin: Kirigami.Units.largeSpacing * 2
        root.view.leftMargin: Kirigami.Units.largeSpacing * 2
    }
}
