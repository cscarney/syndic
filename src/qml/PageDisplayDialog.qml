import QtQuick 2.12
import QtQuick.Window 2.12
import org.kde.kirigami 2.7 as Kirigami

Window {
    id: root
    property string pageSource;
    property var pageProps: ({})

    modality: Qt.WindowModal
    flags: Qt.Dialog

    Kirigami.PageRow {
        id: pageRow
        anchors.fill: parent
    }

    Component.onCompleted: {
        pageRow.push(pageSource, pageProps);
    }
}
