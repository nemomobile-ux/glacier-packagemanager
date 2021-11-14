/*
 * Copyright (C) 2018-2021 Chupligin Sergey <neochapay@gmail.com>
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

import QtQuick 2.6

import QtQuick.Controls 1.0
import QtQuick.Controls.Nemo 1.0
import QtQuick.Controls.Styles.Nemo 1.0

import "../components"

Page {
    id: mainPage
    property var searchPackages: []

    headerTools: HeaderToolsLayout {
        id: tools
        title: qsTr("Package manager")

        tools: [
            ToolButton {
                id: refreshButton
                iconSource: "image://theme/refresh"
                onClicked: pageStack.push(Qt.resolvedUrl("/usr/share/glacier-packagemanager/qml/pages/UpdatesPage.qml"));
                showCounter: counterValue != 0
                counterValue: 0
            }
        ]
    }

    Item{
        id: rootView
        anchors.fill: parent

        Column{
            id: rootColumn
            spacing:Theme.itemSpacingLarge
            width: parent.width - Theme.itemSpacingLarge*2
            height: parent.height - Theme.itemSpacingLarge *2

            anchors{
                top: parent.top
                topMargin: Theme.itemSpacingLarge
                left: parent.left
                leftMargin: Theme.itemSpacingLarge
            }

            TextField{
                id: searchLine
                width: parent.width
                onTextChanged: {
                    pkgDb.searchPackages(searchLine.text)
                }
            }

            ListView{
                id: searchListView
                width: parent.width
                height: parent.height - searchLine.height
                clip: true
                model: searchPackages
                delegate: PackageListDelegate{
                    pkg: modelData
                }
            }
        }
    }

    Connections{
        target: pkgDb
        function onSearchPackagesReady(packages) {
            searchPackages = packages
        }

        function onGetUpdatesReady(packages) {
            refreshButton.counterValue = packages.length
        }

        function onDbRefreshed() {
            pageStack.pop()
            refreshButton.counterValue = packages.length
            if(searchLine.text != "") {
                pkgDb.searchPackages(searchLine.text)
            }
        }
    }

    Component.onCompleted: {
        pkgDb.getUpdates();
    }
}

