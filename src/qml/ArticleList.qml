import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import org.kde.kirigami 2.9 as Kirigami

ListView {
    id: articleList
    clip: true

    delegate: Kirigami.AbstractListItem {
        anchors {
            left: parent.left
            right: parent.right
        }

        text: headline
        padding: 10
        contentItem: ArticleListEntry { }
        
        property var data: model

        highlighted: ListView.isCurrentItem
        onClicked: {
            articleList.currentIndex = model.index
        }
    }
}
