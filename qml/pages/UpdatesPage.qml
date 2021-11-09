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

import Nemo.Dialogs 1.0

import org.glacier.packagemanager 1.0

Page {
    id: mainPage

    headerTools: HeaderToolsLayout {
        id: tools
        title: updatesListView.count > 0
               ? qsTr("Updates: %1").arg(updatesListView.count)
               : qsTr("Updates")

        showBackButton: action == "update" ? false : true
    }

    UpdatesListModel{
        id: updateListModel

        onUpdatesReady: {
            if(updatesListView.count == 0) {
                updatesLabel.text = qsTr("System is up to date")
            } else {
                updatesLabel.visible = false
                updatesListView.visible = true
                startUpdateButton.visible = true
            }
        }
    }

    Rectangle{
        id: updatesView
        width: parent.width
        height: parent.height

        color: "transparent"

        visible: !updateProgress.visible

        ListView{
            id: updatesListView
            model: updateListModel
            width: parent.width
            height: parent.height-startUpdateButton.height-Theme.itemSpacingSmall

            anchors.top: parent.top
            visible: false
            clip: true

            delegate: ListViewItemWithActions{
                label: name
                description: summary
                showNext: false
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

            onClicked: {
                packageManager.installUpdates(updateListModel.getSelectedPackagesIds());
            }

        }

        Label{
            id: updatesLabel
            anchors.centerIn: parent
            text: qsTr("Loading updates list")
        }
    }

    Rectangle{
        id: updateProgress

        anchors.centerIn: parent
        width: parent.width
        height: childrenRect.height

        color: "transparent"

        visible: false

        ProgressBar{
            id: refreshProgressBar

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
            text: qsTr("Updating...")

            anchors{
                top: refreshProgressBar.bottom
                topMargin: Theme.itemSpacingLarge
                horizontalCenter: parent.horizontalCenter
            }
        }
    }

    Dialog{
        id: simpleDialog
        acceptText: qsTr("Ok")
        headingText: qsTr("Packages updated")

        inline: false

        icon: "image://theme/exclamation-triangle"

        onAccepted: {
            back();
        }
    }

    Connections{
        target: packageManager
        onUpdatePackagesStarted: {
            updateProgress.visible = true
        }
        onUpdatePackagesProgress: {
            refreshProgressBar.value = percentage;
        }
        onUpdatePackagesFinished: {
            simpleDialog.open();
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
