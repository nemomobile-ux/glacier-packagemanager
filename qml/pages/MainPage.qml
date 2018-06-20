/*
 * Copyright (C) 2018 Chupligin Sergey <neochapay@gmail.com>
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

Page {
    id: mainPage

    headerTools: HeaderToolsLayout {
        id: tools
        title: qsTr("Package manager")

        tools: [
            ToolButton {
                iconSource: "image://theme/refresh"
                onClicked: packageManager.refreshRepos(true);
                visible: (action !== "remove")
            }
        ]
    }

    Item{
        id: refreshRepoIndicator

        anchors.centerIn: parent
        width: parent.width
        height: childrenRect.height

        ProgressBar{
            id: refreshRepoProgressBar

            width: mainPage.width - Theme.itemSpacingLarge*2
            anchors{
                left: parent.left
                leftMargin: Theme.itemSpacingLarge
            }

            indeterminate: true
            minimumValue: 0
            maximumValue: 100
        }

        Label{
            id: refreshRepoLabel
            text: qsTr("Updating repo...")

            anchors{
                top: refreshRepoProgressBar.bottom
                topMargin: Theme.itemSpacingLarge
                horizontalCenter: parent.horizontalCenter
            }
        }

    }

    Component.onCompleted: {
        packageManager.refreshRepos(false);
    }

    Connections{
        target: packageManager
        onRefreshReposFinished: {
            refreshRepoProgressBar.value = 100
            if(action !== "install"){
                packageManager.getUpdates()
            }
        }

        onRefreshReposProgress: {
            refreshRepoProgressBar.value = percentage
        }

        onRefreshReposStarted: {
            refreshRepoProgressBar.value = 0
        }

        onUpdatesReady: {
            if(action === "update") {
                pageStack.replace(Qt.resolvedUrl("/usr/share/glacier-packagemanager/qml/pages/UpdatesPage.qml"));
            } else if (action === "install") {
                console.log("Try install "+pkgname)
                pageStack.replace(Qt.resolvedUrl("/usr/share/glacier-packagemanager/qml/pages/InstallPackagePage.qml"));
            }

            console.log("ACTION IS "+action);
        }
    }
}

