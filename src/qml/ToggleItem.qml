import com.rocksandpaper.syndic.templates 1.0 as T
import QtQuick 2.15
import org.kde.kirigami 2.14 as Kirigami

T.ToggleItem {
    id: root

    PropertyAnimation {
        id: animateHeight
        target: toggleItem
        property: "implicitHeight"
        duration: Kirigami.Units.shortDuration
        easing.type: Easing.InOutCubic
    }

    Connections {
        target: toggleItem.activeItem
        function onImplicitHeightChanged() {
            if (animateHeight.running) {
                animateHeight.stop()
                animateHeight.from = root.implicitHeight
                animateHeight.to = target.implicitHeight
                animateHeight.start()
            } else {
                toggleItem.implicitHeight = target.implicitHeight
            }
        }
    }

    onActiveItemChanged: function() {
        if (toggleItem.implicitHeight > 0) {
            animateHeight.stop()
            animateHeight.from = root.implicitHeight
            animateHeight.to = activeItem.implicitHeight
            animateHeight.start()
        } else {
            toggleItem.implicitHeight = activeItem.implicitHeight
        }
    }
}
