import QtQuick 2.15
import QtQuick.Controls 2.15

// combo box that expands to fit the widest item
// TODO Qt6: We can get rid of this and use implicitContentWidthPolicy: WidestTextWhenCompleted
ComboBox {
    id: root

    readonly property FontMetrics fm: FontMetrics {
        font: root.font
    }

    Component.onCompleted: {
        let widestItem = 0;
        for(let i=0; i<root.count; i++) {
            let width = fm.advanceWidth(root.textAt(i))
            widestItem = Math.max(widestItem, width);
        }
        root.contentItem.implicitWidth = widestItem + leftPadding + rightPadding;
    }
}
