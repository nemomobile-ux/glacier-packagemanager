/*
 * Copyright (C) 2021 Chupligin Sergey <neochapay@gmail.com>
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

import Nemo.Dialogs 1.0

import "../components"

Page {
    id: orphansPage
    property var orphansListModel: []

    headerTools: HeaderToolsLayout {
        id: tools
        title: qsTr("Orphaned packages")
        showBackButton: true
    }

    Item{
        id: orphansView
        anchors.fill: parent

        ListView{
            id: orphansListView
            model: orphansListModel
            width: parent.width
            height: parent.height-startCleanupButton.height-Theme.itemSpacingSmall

            anchors.top: parent.top
            visible: orphansListModel.length > 0
            clip: true

            delegate: PackageListDelegate{
                pkg: modelData
            }
        }

        Button{
            id: startCleanupButton
            width: parent.width
            height: Theme.itemHeightExtraLarge
            text: qsTr("Cleanup")

            anchors.bottom: parent.bottom
            visible: orphansListModel.length > 0

            onClicked: {
                var toRemove = []
                for(var i = 0; i < orphansListModel.length; i++) {
                    toRemove.push(orphansListModel[i].name)
                    pkgTa.remove(toRemove)
                }
            }
        }

        Label{
            id: orphansLabel
            anchors.centerIn: parent
            text: qsTr("Loading orphaned packages list")
            visible: orphansListModel.length == 0
        }
    }

    Component.onCompleted: {
        pkgDb.getOrphansPackages();
    }

    Connections{
        target: pkgDb

        function onGetOrphansPackagesReady(packages) {
            orphansPage.orphansListModel = packages
        }
    }
}
