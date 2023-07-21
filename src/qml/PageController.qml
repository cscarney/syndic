import QtQuick 2.12
import com.rocksandpaper.syndic 1.0
import org.kde.kirigami 2.15 as Kirigami

QtObject {
    id: root
    required property Kirigami.PageRow pageRow
    property string firstPageComponent: ""
    property var firstPageData: null

    property list<QtObject> data: [
        Connections {
            target: root.pageRow

            function onPagePushed(page) {
                page.PageControl.setPageController(root);
                pushNextPage(page);
            }

            function onPageRemoved(page) {
                page.PageControl.setPageController(null);
            }
        }
    ]

    Component.onCompleted: {
        pushFirstPage();
        root.firstPageComponentChanged.connect(pushFirstPage);
        root.firstPageDataChanged.connect(pushFirstPage);
    }

    function pushNextPage(thisPage) {
        let pageComponent = thisPage.PageControl.nextPage.componentName
        if (pageComponent) {
            let pageData = thisPage.PageControl.nextPage.pageData
            root.pageRow.push(pageComponent, pageData)
        }
    }

    function pushFirstPage(pageComponent, pageData) {
        if (!pageComponent) {
            pageComponent = root.firstPageComponent;
            pageData = root.firstPageData;
        }
        if (pageRow.depth) {
            root.pageRow.currentIndex = 0;
            root.pageRow.replace(pageComponent, pageData);
        } else {
            root.pageRow.push(pageComponent, pageData);
        }
    }

    function sync(page) {
        root.pageRow.pop(page);
        pushNextPage(page);
    }
}
