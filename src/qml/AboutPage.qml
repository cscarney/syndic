import QtQuick 2.0
import org.kde.kirigami 2.7 as Kirigami

Kirigami.AboutPage {
    property bool keepDrawerOpen: true
    aboutData: ({
        displayName: "FeedKeeper",
        productName: "feedkeeper",
        componentName: "feedkeeper",
        shortDescription: "An Elegant RSS Reader",
        homepage: "",
        bugAddress: "feedkeeper@runningincircles.com",
        version: "1.0",
        otherText: "",
        authors: [
            {
                name: "Connor Carney",
                task: "",
                emailAddress: "connor@runningincircles.com",
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
        copyrightStatement: "©2020 Connor Carney",
        desktopFileName: "com.connorcarney.feedkeeper"
    })


    readonly property string gplText: "This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
 "
}
