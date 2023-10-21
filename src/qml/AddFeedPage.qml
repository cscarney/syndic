/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick 2.0
import org.kde.kirigami 2.7 as Kirigami
import com.rocksandpaper.syndic 1.0

AbstractFeedEditorPage {
    id: root
    required property Item pageRow
    property bool previewOpen: false
    property bool keepDrawerOpen: true
    title: qsTr("Add Content")

    provisionalFeed: ProvisionalFeed {
        updateMode: Feed.InheritUpdateMode
        updateInterval: globalSettings.updateInterval
        expireMode: Feed.InheritUpdateMode
        expireAge: globalSettings.expireAge

        onUrlStringEdited: {
            previewOpen = false
        }
    }

    actions: [
        Kirigami.Action {
            id: previewAction
            text: qsTr("Preview…")
            icon.name: "document-preview"
            checkable: true
            checked: previewOpen
            onToggled: previewOpen = checked
        }
    ]

    onPreviewOpenChanged: {
        if (previewOpen) {
            provisionalFeed.updater.start()
            pageRow.currentIndex = Kirigami.ColumnView.index
            pageRow.push("qrc:/qml/ArticleList/FeedPreviewPage.qml",
                         {provisionalFeed: root.provisionalFeed,
                          pageRow: root.pageRow});
            previewOpen = true;
        } else {
            pageRow.pop(this);
        }
    }

    Keys.onReturnPressed: previewOpen = true
    Keys.onEnterPressed: previewOpen = true
}
