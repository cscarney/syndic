/*
 *  SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.15
import QtQuick.Templates 2.15 as T
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.19
import org.kde.kirigami.templates 2.2 as KT

/**
 * ScrollablePage is a Page that holds scrollable content, such as ListViews.
 * Scrolling and scrolling indicators will be automatically managed.
 *
 * @code
 * ScrollablePage {
 *     id: root
 *     //The rectangle will automatically be scrollable
 *     Rectangle {
 *         width: root.width
 *         height: 99999
 *     }
 * }
 * @endcode
 *
 * @warning Do not put a ScrollView inside of a ScrollablePage; children of a ScrollablePage are already inside a ScrollView.
 *
 * Another behavior added by this class is a "scroll down to refresh" behavior
 * It also can give the contents of the flickable to have more top margins in order
 * to make possible to scroll down the list to reach it with the thumb while using the
 * phone with a single hand.
 *
 * Implementations should handle the refresh themselves as follows
 *
 * @code
 * Kirigami.ScrollablePage {
 *     id: view
 *     supportsRefreshing: true
 *     onRefreshingChanged: {
 *         if (refreshing) {
 *             myModel.refresh();
 *         }
 *     }
 *     ListView {
 *         // NOTE: MyModel doesn't come from the components,
 *         // it's purely an example on how it can be used together
 *         // some application logic that can update the list model
 *         // and signals when it's done.
 *         model: MyModel {
 *             onRefreshDone: view.refreshing = false;
 *         }
 *         delegate: BasicListItem {}
 *     }
 * }
 * [...]
 * @endcode
 *
 */
Page {
    id: root

    /**
     * \property bool ScrollablePage::refreshing
     * If true the list is asking for refresh and will show a loading spinner.
     * it will automatically be set to true when the user pulls down enough the list.
     * This signals the application logic to start its refresh procedure.
     * The application itself will have to set back this property to false when done.
     */
    property alias refreshing: scrollView.refreshing

    /**
     * \property bool ScrollablePage::supportsRefreshing
     * If true the list supports the "pull down to refresh" behavior.
     * By default it is false.
     */
    property alias supportsRefreshing: scrollView.supportsRefreshing

    /**
     * \property QtQuick.Flickable ScrollablePage::flickable
     * The main Flickable item of this page.
     */
    property alias flickable: scrollView.flickableItem

    /**
     * \property Qt.ScrollBarPolicy ScrollablePage::verticalScrollBarPolicy
     * The vertical scrollbar policy.
     */
    property alias verticalScrollBarPolicy: scrollView.verticalScrollBarPolicy

    /**
     * \property Qt.ScrollBarPolicy ScrollablePage::horizontalScrollBarPolicy
     * The horizontal scrollbar policy.
     */
    property alias horizontalScrollBarPolicy: scrollView.horizontalScrollBarPolicy

    /**
     * The main content Item of this page.
     * In the case of a ListView or GridView, both contentItem and flickable
     * will be a pointer to the ListView (or GridView).
     * @note This can't be contentItem as Page's contentItem is final.
     */
    default property QtObject mainItem

    /**
     * If true, and if flickable is an item view, like a ListView or
     * a GridView, it will be possible to navigate the list current item
     * to next and previous items with keyboard up/down arrow buttons.
     * Also, any key event will be forwarded to the current list item.
     * default is true.
     */
    property bool keyboardNavigationEnabled: true

    contentHeight: root.flickable.contentHeight
    implicitHeight: ((header && header.visible) ? header.implicitHeight : 0) + ((footer && footer.visible) ? footer.implicitHeight : 0) + contentHeight + topPadding + bottomPadding
    implicitWidth: root.flickable.contentItem ? root.flickable.contentItem.implicitWidth : contentItem.implicitWidth + leftPadding + rightPadding

    Theme.inherit: false
    Theme.colorSet: flickable && flickable.hasOwnProperty("model") ? Theme.View : Theme.Window

    clip: true
    contentItem: RefreshableScrollView {
        id: scrollView
        //NOTE: here to not expose it to public api
        property QtObject oldMainItem
        page: root
        clip: true
        topPadding: contentItem === flickableItem ? 0 : root.topPadding
        leftPadding: root.leftPadding
        rightPadding: root.rightPadding
        bottomPadding: contentItem === flickableItem ? 0 : root.bottomPadding
        anchors {
            top: (root.header && root.header.visible)
                    ? root.header.bottom
                    //FIXME: for now assuming globalToolBarItem is in a Loader, which needs to be got rid of
                    : (globalToolBarItem && globalToolBarItem.parent && globalToolBarItem.visible
                        ? globalToolBarItem.parent.bottom
                        : parent.top)
            bottom: (root.footer && root.footer.visible) ? root.footer.top : parent.bottom
            left: parent.left
            right: parent.right
        }
    }

    anchors.topMargin: 0

    Keys.forwardTo: root.keyboardNavigationEnabled && root.flickable
                        ? (("currentItem" in root.flickable) && root.flickable.currentItem ?
                           [ root.flickable.currentItem, root.flickable ] : [ root.flickable ])
                        : []

    //HACK to get the mainItem as the last one, all the other eventual items as an overlay
    //no idea if is the way the user expects
    onMainItemChanged: {
        if (mainItem instanceof Item) {
            scrollView.contentItem = mainItem
            mainItem.focus = true
        } else if (mainItem instanceof T.Drawer) {
            //don't try to reparent drawers
            return;
        } else if (mainItem instanceof KT.OverlaySheet) {
            //reparent sheets
            if (mainItem.parent === root || mainItem.parent === null) {
                mainItem.parent = root;
            }
            root.data.push(mainItem);
            return;
        }

        if (scrollView.oldMainItem && scrollView.oldMainItem instanceof Item
            && (typeof applicationWindow === 'undefined'|| scrollView.oldMainItem.parent !== applicationWindow().overlay)) {
            scrollView.oldMainItem.parent = overlay
        }
        scrollView.oldMainItem = mainItem
    }
}
