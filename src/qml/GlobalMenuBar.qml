/**
 * SPDX-FileCopyrightText: 2026 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick
import Qt.labs.platform as Platform

/**
 * Global Menu Bar
 */
Platform.MenuBar {
    id: root

    /// The Kirigami.PageRow whose items are scanned for page actions.
    property var pageStack

    /// The always-alive AppActions instance. App-level menu items target these
    /// directly -- no resolution needed, they never go away.
    property var appActions

    /// Invoked by View > Back. Supplied by main.qml, which owns the navigation
    /// logic (a plain function value, not a binding).
    property var goBack: function() {}

    readonly property bool canGoBack:
        !!pageStack && pageStack.leadingVisibleItem !== pageStack.items[0]

    /**
     * Find the live action carrying `id`, or null.
     */
    function resolve(id) {
        if (!pageStack) {
            return null;
        }
        const pages = pageStack.items;
        for (let i = pages.length - 1; i >= 0; --i) {
            const page = pages[i];
            // separators and placeholders have no actions
            if (!page || page.actions === undefined || page.actions === null) {
                continue;
            }
            const actions = page.actions;
            for (let j = 0; j < actions.length; ++j) {
                const action = actions[j];
                if (action && action.actionId === id) {
                    return action;
                }
            }
        }
        return null;
    }

    /**
     * A menu item that forwards to a resolved PageAction (§3.4).
     *
     * text/shortcut/checkable/role are declared statically per instance, so
     * the item keeps its label and accelerator while its page is unloaded;
     * enabled/visible/checked are read-through mirrors of the target, and
     * activation only ever calls trigger(). Nothing is written back to the
     * target, and icons are deliberately not propagated -- macOS menus don't
     * use them.
     */
    component MenuAction: Platform.MenuItem {
        /// The actionId this item forwards to.
        property string targetId

        // (pageStack.items, ...) makes the item list a real binding dependency,
        // so the target re-resolves as pages are pushed and popped.
        readonly property var target: (root.pageStack ? root.pageStack.items : null,
                                       root.resolve(targetId))

        /// One-way mirror; never assigned from here except to restore itself.
        readonly property bool targetChecked: !!target && target.checked === true

        enabled: !!target && target.enabled
        checked: targetChecked

        onTriggered: {
            if (target) {
                target.trigger();
            }
            // MenuItem toggles its own `checked` on activation, which
            // would destroy the mirror above.
            checked = Qt.binding(() => targetChecked);
        }
    }

    /**
     * A menu item bound to an always-alive AppActions action. No resolution,
     * and the label can be read straight off the action, so there is no
     * duplicated qsTr() to worry about for these.
     */
    component AppMenuAction: Platform.MenuItem {
        property var action
        text: action ? action.text : ""
        enabled: !!action && action.enabled
        onTriggered: if (action) action.trigger()
    }

    Platform.Menu {
        title: qsTr("&File")

        AppMenuAction {
            action: root.appActions ? root.appActions.addContent : null
            shortcut: "Ctrl+N"
        }

        Platform.MenuSeparator { }

        MenuAction {
            targetId: "feed.refresh"
            text: qsTr("Refresh")
            shortcut: "Ctrl+R"
        }

        MenuAction {
            targetId: "feed.cancel"
            text: qsTr("Cancel Updates")
            shortcut: "Ctrl+."
        }

        Platform.MenuSeparator { }

        AppMenuAction {
            action: root.appActions ? root.appActions.importOpml : null
        }

        AppMenuAction {
            action: root.appActions ? root.appActions.exportOpml : null
        }

        Platform.MenuSeparator {}

        AppMenuAction {
            role: Platform.MenuItem.PreferencesRole
            text: qsTr("Configure %1...").arg(Qt.application.displayName)
            shortcut: "Ctrl+Shift+,"
            action: root.appActions ? root.appActions.settings : null
        }

        Platform.MenuSeparator {}

        Platform.MenuItem {
            role: Platform.MenuItem.QuitRole
            text: qsTr("&Quit")

            // Close the window rather than Qt.quit(): Application only decides
            // between quitting and staying resident as a background notifier
            // once the last window has closed (application.cpp:213).
            onTriggered: {
                if (root.window) {
                    root.window.close();
                } else {
                    Qt.quit();
                }
            }
        }

    }

    Platform.Menu {
        title: qsTr("&Edit")

        MenuAction {
            targetId: "feed.edit"
            text: qsTr("Edit Feed…")
        }

        MenuAction {
            targetId: "editor.delete"
            //: menu entry; the editor page's button says the shorter "Delete"
            text: qsTr("Delete Feed")
        }
    }

    Platform.Menu {
        title: qsTr("&View")

        MenuAction {
            targetId: "feed.hideRead"
            text: qsTr("Hide Read Articles")
            checkable: true
        }

        MenuAction {
            targetId: "article.toggleWebContent"
            text: qsTr("Show Web Content")
            checkable: true
        }

        MenuAction {
            targetId: "article.reloadWebContent"
            text: qsTr("Reload Web Content")
            shortcut: "Shift+Ctrl+R"
        }

        Platform.MenuSeparator { }

        Platform.MenuItem {
            text: qsTr("Back")
            shortcut: "Ctrl+["
            enabled: root.canGoBack
            onTriggered: root.goBack()
        }
    }

    Platform.Menu {
        title: qsTr("&Article")

        MenuAction {
            targetId: "article.openInBrowser"
            //: menu entry; the toolbar button says the shorter "Open"
            text: qsTr("Open in Browser")
            shortcut: "Ctrl+O"
        }

        MenuAction {
            targetId: "article.share"
            text: qsTr("Share")
        }

        Platform.MenuSeparator { }

        MenuAction {
            targetId: "article.star"
            //: menu entry (a verb); the toolbar button says "Starred"
            text: qsTr("Star")
            shortcut: "Ctrl+L"
            checkable: true
        }

        MenuAction {
            targetId: "article.keepUnread"
            //: as in, don't mark this article as read
            text: qsTr("Keep Unread")
            shortcut: "Shift+Ctrl+U"
            checkable: true
        }

        MenuAction {
            targetId: "feed.markAllRead"
            //: menu entry; the toolbar button says the shorter "Mark All Read"
            text: qsTr("Mark All Items as Read")
            shortcut: "Shift+Ctrl+A"
        }

        Platform.MenuSeparator { }

        MenuAction {
            targetId: "article.next"
            text: qsTr("Next Article")
        }

        MenuAction {
            targetId: "article.previous"
            text: qsTr("Previous Article")
        }
    }

    Platform.Menu {
        title: qsTr("&Help")
        AppMenuAction {
            role: Platform.MenuItem.AboutRole
            action: root.appActions ? root.appActions.about : null
        }
    }
}
