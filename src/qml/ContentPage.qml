import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import org.kde.kirigami 2.7 as Kirigami
import ContentModel 1.0
import Qt.labs.settings 1.1

Kirigami.ScrollablePage {
    id: root
    property var item
    property var nextItem: function() {}
    property var previousItem: function() {}
    signal suspendAnimations;

    Connections {
        target: item.article
        function onGotContent(content) {
            contentModel.text = content;
        }
    }

    Component {
        id: textComponent
        ColumnLayout {
            width: root.flickable.width
            TextEdit {
                text: block.text
                color: Kirigami.Theme.textColor
                selectionColor: Kirigami.Theme.highlightColor
                selectedTextColor: Kirigami.Theme.highlightedTextColor
                wrapMode: Text.Wrap
                textFormat: Text.RichText
                readOnly: true
                selectByMouse: true
                font.family: "serif"
                font.pointSize: settings.fontSize
                Layout.preferredWidth: parent.width
            }
        }
    }

    Component {
        id: imageComponent
        ColumnLayout {
            width: root.flickable.width
            Image {
                source: block.src
                property real scaleToFit: Math.min(parent.width, sourceSize.width) / sourceSize.width
                Layout.preferredWidth: sourceSize.width * scaleToFit
                Layout.preferredHeight: sourceSize.height * scaleToFit
                Layout.alignment: Qt.AlignHCenter
                Layout.bottomMargin: settings.fontSize
                fillMode: Image.PreserveAspectFit
            }
        }
    }

    Settings {
        id: settings
        category: "ArticlePage"
        property var fontSize
    }

    Flickable {
        contentHeight: listView.height
        Repeater {
            id: listView
            model: ContentModel {
                id: contentModel
            }
            delegate: Loader {
                property var block: model.block
                sourceComponent: {
                    if (block.delegateName==="TextBlock") {
                        return textComponent;
                    } else {
                        return imageComponent;
                    }
                }
            }
        }
    }

    Component.onCompleted: {
        item.article.requestContent();
    }
}
