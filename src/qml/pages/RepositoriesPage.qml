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

Page {
    id: repositoriesPage

    property var repositoriesListModel: []

    headerTools: HeaderToolsLayout {
        id: tools
        title: qsTr("Repositories")

        tools: [
            ToolButton {
                id: addRepoButton
                iconSource: "image://theme/plus"
                onClicked: addRepoDialog.open()
            }
        ]
    }

    ListView {
        id: reposListView
        anchors.fill: parent
        anchors.margins: Theme.itemSpacingLarge
        spacing: Theme.itemSpacingSmall
        clip: true
        model: repositoriesListModel

        delegate: ListViewItemWithActions {
            width: reposListView.width
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
        anchors.centerIn: parent
        visible: repositoriesListModel.length === 0
        text: qsTr("No repositories found")
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
        pkgDb.listRepositories()
    }
}
