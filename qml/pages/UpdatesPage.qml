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

import Nemo.Dialogs 1.0

import "../components"

Page {
    id: updatesPage
    property var updateListModel: []

    headerTools: HeaderToolsLayout {
        id: tools
        title: updatesListView.count > 0
               ? qsTr("Updates: %1").arg(updatesListView.count)
               : qsTr("Updates")

        showBackButton: action == "update" ? false : true
    }

    Item{
        id: updatesView
        anchors.fill: parent

        ListView{
            id: updatesListView
            model: updateListModel
            width: parent.width
            height: parent.height-startUpdateButton.height-Theme.itemSpacingSmall

            anchors.top: parent.top
            visible: updateListModel.length > 0
            clip: true

            delegate: PackageListDelegate{
                pkg: modelData
            }
        }

        Button{
            id: startUpdateButton
            width: parent.width
            height: Theme.itemHeightExtraLarge

            enabled: updatesListView.count > 0
            text: qsTr("Update packages")

            anchors.bottom: parent.bottom
            visible: false
        }

        Label{
            id: updatesLabel
            anchors.centerIn: parent
            text: qsTr("Loading updates list")
            visible: updateListModel.length == 0
        }
    }

    Component.onCompleted: {
        pkgDb.getUpdates();
    }

    Connections{
        target: pkgDb

        function onGetUpdatesReady(packages) {
            updatesPage.updateListModel = packages
        }
    }

    function back(){
        if(action === "update"){
            Qt.quit()
        } else {
            pageStack.pop()
        }
    }
}
