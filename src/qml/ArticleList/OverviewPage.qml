import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import com.rocksandpaper.syndic
import org.kde.kirigami as Kirigami
import ".."

AbstractArticleListPage {
    id: root
    title: qsTr("Highlights")
    model: HighlightsModel {
        context: feedContext
    }
    bottomPadding: Kirigami.Units.largeSpacing * 2
    automaticOpen: false

    delegate: Kirigami.AbstractCard {
        id: itemCard
        required property var ref
        required property int index
        property Article article: ref.article
        property ArticleSummary articleSummary: ArticleSummary {
            article: ref
        }

        visible: articleSummary?.finished ?? false
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
                text: articleSummary.firstParagraph
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

        onClicked: {
            root.selectIndex(index)
        }
    }

    actions: [
        Kirigami.Action {
            text: qsTr("Refresh")
            icon.name: "view-refresh"
            onTriggered: {
                root.model.requestUpdate();
            }
        }
    ]

    Binding {
        root.view.spacing: Kirigami.Units.largeSpacing * 2
        root.view.topMargin: root.view.spacing
        root.view.rightMargin: Kirigami.Units.largeSpacing * 2
        root.view.leftMargin: Kirigami.Units.largeSpacing * 2
    }
}
