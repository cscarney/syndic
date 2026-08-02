/**
 * SPDX-FileCopyrightText: 2026 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick
import QtQuick.Dialogs
import org.kde.kirigami as Kirigami
import com.rocksandpaper.syndic 1.0

/**
 * Application-level actions, instantiated once by main.qml.
 *
 * Unlike page actions these are always alive, so anything that outlives a
 * page -- the global drawer, the settings page, and later a platform menu
 * bar -- can reference the same instances directly instead of duplicating
 * the behavior.
 */
QtObject {
    id: root

    /// Pushes a utility page by URL. Supplied by main.qml, which owns the
    /// page stack and the navigation state that has to be reset first.
    required property var pushUtilityPage

    /// The page row that pushed pages are given as their `pageRow`.
    required property var pageRow

    /// Model handed to the settings page.
    required property FeedListModel feedListModel

    readonly property Kirigami.Action addContent: Kirigami.Action {
        text: qsTr("Add Content")
        icon.name: "list-add"
        onTriggered: {
            root.pushUtilityPage("qrc:/qml/AddFeedPage.qml", {pageRow: root.pageRow})
        }
    }

    readonly property Kirigami.Action settings: Kirigami.Action {
        text: qsTr("Settings")
        icon.name: "settings-configure"
        onTriggered: {
            root.pushUtilityPage("qrc:/qml/SettingsPage.qml",
                                 {feedListModel: root.feedListModel, appActions: root})
        }
    }

    readonly property Kirigami.Action about: Kirigami.Action {
        text: qsTr("About %1").arg(Qt.application.displayName)
        icon.name: "help-about"
        onTriggered: {
            root.pushUtilityPage("qrc:/qml/AboutPage.qml")
        }
    }

    readonly property Kirigami.Action importOpml: Kirigami.Action {
        text: qsTr("Import OPML…")
        icon.name: "document-import"
        onTriggered: {
            root.openFileDialog(FileDialog.OpenFile, function(file) {
                feedContext.importOpml(file);
            });
        }
    }

    readonly property Kirigami.Action exportOpml: Kirigami.Action {
        text: qsTr("Export OPML…")
        icon.name: "document-export"
        onTriggered: {
            root.openFileDialog(FileDialog.SaveFile, function(file) {
                feedContext.exportOpml(file);
            });
        }
    }

    /// Lazily instantiated so the dialog only exists once it is asked for.
    readonly property Loader fileDialogLoader: Loader { }

    readonly property Component fileDialogComponent: Component {
        FileDialog {
            property var acceptedFunc: function() {}
            onAccepted: acceptedFunc();
        }
    }

    function openFileDialog(fileMode, onFileSelected) {
        fileDialogLoader.sourceComponent = fileDialogComponent;
        const dialog = fileDialogLoader.item;
        dialog.fileMode = fileMode;
        dialog.acceptedFunc = function() {
            onFileSelected(dialog.selectedFile);
        }
        dialog.open();
    }
}
