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
import Nemo.Dialogs
import Nemo.Configuration

import Glacier.Controls.Settings 1.0
import Glacier.PackageManager 2.0

Page {
    id: packageManagerPage

    property var repositoriesListModel: []

    headerTools: HeaderToolsLayout {
        showBackButton: true
        title: qsTr("Package manager")
        tools: [
            ToolButton {
                iconSource: "image://theme/plus"
                onClicked: addRepoDialog.open()
            }
        ]
    }

    ConfigurationValue {
        id: connectionPolicyValue
        key: "/home/glacier/packagemanager/updateConnectionPolicy"
        defaultValue: "wifiEthernet"
    }

    PackageDatabase {
        id: pkgDb
    }

    ListModel {
        id: policyModel
    }

    Flickable {
        anchors.fill: parent
        contentHeight: contentColumn.height + Theme.itemSpacingLarge * 2
        clip: true

        Column {
            id: contentColumn
            width: parent.width - Theme.itemSpacingLarge * 2
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: Theme.itemSpacingLarge
            spacing: Theme.itemSpacingLarge

            Label {
                text: qsTr("Update checks")
                font.bold: true
                width: parent.width
            }

            GlacierRoller {
                id: policyRoller
                width: parent.width
                label: qsTr("Allow updates on")
                model: policyModel
                delegate: GlacierRollerItem {
                    Text {
                        text: name
                        color: Theme.textColor
                        font.pixelSize: Theme.fontSizeMedium
                        font.bold: (policyRoller.activated && policyRoller.currentIndex === index)
                        verticalAlignment: Text.AlignVCenter
                        height: policyRoller.itemHeight
                    }
                }
                onSelect: {
                    connectionPolicyValue.value = policyModel.get(currentIndex).value
                }
            }

            Label {
                text: qsTr("Repositories")
                font.bold: true
                width: parent.width
            }

            Repeater {
                model: repositoriesListModel
                delegate: ListViewItemWithActions {
                    width: contentColumn.width
                    height: Theme.itemHeightLarge
                    label: modelData.name !== "" ? modelData.name : modelData.id
                    description: modelData.url + " [" + modelData.backend + "]"
                    showNext: false
                    iconVisible: false

                    actions: [
                        ActionButton {
                            iconSource: modelData.enabled ? "image://theme/checkbox-checked" : "image://theme/checkbox-unchecked"
                            onClicked: pkgDb.setRepositoryEnabled(modelData.id, !modelData.enabled, modelData.backend)
                        },
                        ActionButton {
                            iconSource: "image://theme/trash"
                            onClicked: pkgDb.removeRepository(modelData.id, modelData.backend)
                        }
                    ]
                }
            }

            Label {
                visible: repositoriesListModel.length === 0
                width: parent.width
                text: qsTr("No repositories found")
                horizontalAlignment: Text.AlignHCenter
            }
        }
    }

    CustomDialog {
        id: addRepoDialog
        headingText: qsTr("Add repository")
        acceptText: qsTr("Add")
        cancelText: qsTr("Cancel")

        content: [
            Column {
                width: addRepoDialog.width - Theme.itemSpacingMedium * 2
                spacing: Theme.itemSpacingSmall

                TextField {
                    id: repoNameField
                    width: parent.width
                    placeholderText: qsTr("Name")
                }

                TextField {
                    id: repoUrlField
                    width: parent.width
                    placeholderText: qsTr("URL or source line")
                }

                TextField {
                    id: repoBackendField
                    width: parent.width
                    placeholderText: qsTr("Backend (apt/rpm)")
                    text: "apt"
                }
            }
        ]

        onAccepted: {
            if (repoUrlField.text.length > 0) {
                pkgDb.addRepository(repoUrlField.text, repoNameField.text, repoBackendField.text)
                repoNameField.text = ""
                repoUrlField.text = ""
            }
        }
    }

    Connections {
        target: pkgDb
        function onListRepositoriesReady(repositories) {
            repositoriesListModel = repositories
        }
        function onDbRefreshed() {
            pkgDb.listRepositories()
        }
    }

    Component.onCompleted: {
        policyModel.append({ name: qsTr("Wi-Fi and Ethernet only"), value: "wifiEthernet" })
        policyModel.append({ name: qsTr("Any connection (including mobile)"), value: "any" })
        for (var i = 0; i < policyModel.count; ++i) {
            if (policyModel.get(i).value === connectionPolicyValue.value) {
                policyRoller.currentIndex = i
                break
            }
        }
        pkgDb.listRepositories()
    }
}
