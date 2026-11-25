/*
 * Copyright (C) 2018-2025 Chupligin Sergey <neochapay@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

import QtQuick
import Nemo
import Nemo.Controls

import "../components"

Page {
    id: mainPage

    property bool startupUpdatesChecked: false

    headerTools: HeaderToolsLayout {
        id: tools
        title: qsTr("Package manager")

        tools: [
            ToolButton {
                id: searchButton
                iconSource: "image://theme/search"
                onClicked: pageStack.push(Qt.resolvedUrl("SearchPage.qml"))
            },
            ToolButton {
                id: refreshButton
                iconSource: "image://theme/refresh"
                onClicked: pageStack.push(Qt.resolvedUrl("UpdatesPage.qml"))
                showCounter: counterValue != 0
                counterValue: 0
            },
            ToolButton {
                id: orphansButton
                iconSource: "image://theme/exclamation-circle"
                onClicked: pageStack.push(Qt.resolvedUrl("OrphansPage.qml"))
                showCounter: counterValue != 0
                counterValue: 0
            },
            ToolButton {
                id: reposButton
                iconSource: "image://theme/folder"
                onClicked: pageStack.push(Qt.resolvedUrl("RepositoriesPage.qml"))
            }
        ]
    }

    Item {
        id: rootView
        anchors.fill: parent

        Column {
            id: rootColumn
            spacing: Theme.itemSpacingLarge
            width: parent.width - Theme.itemSpacingLarge * 2
            height: parent.height - Theme.itemSpacingLarge * 2

            anchors {
                top: parent.top
                topMargin: Theme.itemSpacingLarge
                left: parent.left
                leftMargin: Theme.itemSpacingLarge
            }

            Label {
                text: qsTr("Last actions:")
                width: parent.width
                wrapMode: Text.WordWrap
                font.bold: true
            }

            ListView {
                id: historyListView
                width: parent.width
                height: parent.height - parent.spacing
                clip: true
                model: typeof historyItemModel !== "undefined" ? historyItemModel : null
                delegate: HistoryItemDelegate {
                    pkgName: name
                    pkgAction: type
                    pkgVersion: version
                    pkgDate: date
                }
            }
        }
    }

    Connections {
        target: pkgDb
        function onGetUpdatesReady(packages) {
            refreshButton.counterValue = packages.length
            if (!mainPage.startupUpdatesChecked) {
                mainPage.startupUpdatesChecked = true
                if (packages.length > 0 || (typeof action !== "undefined" && action === "update")) {
                    pageStack.push(Qt.resolvedUrl("UpdatesPage.qml"))
                }
            }
        }

        function onGetOrphansPackagesReady(packages) {
            orphansButton.counterValue = packages.length
        }

        function onDbRefreshed() {
            if (pageStack.depth > 1) {
                pageStack.pop()
            }
            pkgDb.getUpdates()
            pkgDb.getOrphansPackages()
        }
    }

    Component.onCompleted: {
        pkgDb.getUpdates()
        pkgDb.getOrphansPackages()
    }
}
