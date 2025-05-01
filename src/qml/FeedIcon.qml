import QtQuick 2.12
import org.kde.kirigami as Kirigami
import com.rocksandpaper.syndic

Kirigami.Icon {
    id: root
    required property Feed feed
    property alias size: root.implicitWidth
    property string iconName: feed.icon.toString()
    property string themeIcon: "feed-subscribe"
    source: iconName.length ? "image://feedicons/"+iconName : themeIcon
    placeholder: "feed-subscribe"
    fallback: "feed-subscribe"
    implicitWidth: Kirigami.Units.iconSizes.smallMedium
    implicitHeight: implicitWidth
}
