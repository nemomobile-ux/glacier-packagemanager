/*
 * Copyright (C) 2026 Chupligin Sergey <neochapay@gmail.com>
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
    id: searchPage

    property var searchPackages: []

    headerTools: HeaderToolsLayout {
        id: tools
        title: qsTr("Search")
        showBackButton: true
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

            TextField {
                id: searchLine
                width: parent.width
                placeholderText: qsTr("Search packages...")
                onTextChanged: {
                    if (searchLine.text !== "") {
                        pkgDb.searchPackages(searchLine.text)
                    } else {
                        searchPackages = []
                    }
                }
            }

            Label {
                width: parent.width
                wrapMode: Text.WordWrap
                visible: searchLine.text === "" && searchPackages.length === 0
                text: qsTr("Type to search for packages")
            }

            ListView {
                id: searchListView
                width: parent.width
                height: parent.height - searchLine.height - rootColumn.spacing
                clip: true
                visible: searchLine.text !== ""
                model: searchPackages ? searchPackages : []
                delegate: PackageListDelegate {
                    pkg: modelData
                }
            }
        }
    }

    Connections {
        target: pkgDb
        function onSearchPackagesReady(packages) {
            searchPackages = packages
        }
    }
}
