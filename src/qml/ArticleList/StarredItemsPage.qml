import QtQuick
import org.kde.kirigami as Kirigami

AbstractFeedPage {
    id: root
    unreadFilter: globalSettings.filterSpecialFeeds && globalSettings.unreadFilter

    Kirigami.Action {
         text: qsTr("Hide Read")
         icon.name: "view-filter"
         checkable: true
         checked: root.unreadFilter
         displayHint: Kirigami.DisplayHint.AlwaysHide
         onCheckedChanged: (checked)=>{
            /* There are two settings here, one for whether to enable the unread
              filter, and one for whether to apply that filter to the starred feed,
              which we expect most users to want unfiltered even if they filter
              normal feeds.

              If the user requests filtering on the starred feed, we need to *both*
              enable the unread filter *and* apply the filter to the starred feed.

              If the user requests to disable the filter on the starred feed, we
              only need to unapply the filter from the starred feed; we don't
              need to disable the unread filter globally.
              */
            if (checked) {
                globalSettings.unreadFilter = true;
                globalSettings.filterSpecialFeeds = true;
            } else {
                globalSettings.filterSpecialFeeds = false;
            }
        }
     }]
}
