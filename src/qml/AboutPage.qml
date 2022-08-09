/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick 2.15
import org.kde.kirigami 2.7 as Kirigami

Kirigami.AboutPage {
    property bool keepDrawerOpen: true
    aboutData: ({
        displayName: Qt.application.displayName,
        productName: "syndic",
        componentName: "syndic",
        shortDescription: "Feed Reader",
        homepage: "http://syndic.rocksandpaper.com/ ",
        bugAddress: "https://github.com/cscarney/syndic/issues/new",
        version: "1.0",
        otherText: "",
        authors: [
            {
                name: "Connor Carney",
                task: "",
                emailAddress: "hello@connorcarney.com",
                webAddress: "",
                ocsUsername: ""
            }
        ],
        credits: [],
        translators: [],
        licenses: [
            {
                name: "GPL v3",
                text: gplText,
                spdx: "GPL-3.0"
            }
        ],
        copyrightStatement: "©2022 Connor Carney",
        desktopFileName: "com.rocksandpaper.syndic"
    })


    readonly property string gplText: "This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
 "
}
