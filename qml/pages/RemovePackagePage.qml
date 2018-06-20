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

Page {
    id: removePage

    headerTools: HeaderToolsLayout {
        id: tools
        title: qsTr("Remove package")
        showBackButton: action !== "remove"
    }

    Rectangle{
        id: removeArea

        width: parent.width
        height: parent.height

        color: "transparent"

        Label{
            id: removeLabel
            text: qsTr("Remove package") + " " + pkgname + "?";
            anchors.centerIn: parent
        }


        ProgressBar{
            id: removeProgressBar

            width: removePage.width - Theme.itemSpacingLarge*2
            anchors{
                left: parent.left
                leftMargin: Theme.itemSpacingLarge
                top: removeLabel.bottom
                topMargin: Theme.itemSpacingLarge
            }

            indeterminate: true
            minimumValue: 0
            maximumValue: 100

            visible: false
        }

        Rectangle{
            id: controlsArea

            width: parent.width
            height: Theme.itemHeightExtraLarge

            anchors.bottom: parent.bottom

            Button{
                id: removeButton
                width: parent.width/2
                height: parent.height
                text: qsTr("Remove")

                anchors.bottom: parent.bottom

                onClicked: {
                    removeProgressBar.visible = true
                    removeProgressBar.value = 0
                    controlsArea.visible = false
                    packageManager.removePackage([pkgname])
                }
            }

            Button{
                id: cancelButton
                width: parent.width/2
                height: parent.height
                text: qsTr("Cancel")

                anchors{
                    left: removeButton.right
                    bottom: parent.bottom
                }

                onClicked: {
                    back();
                }
            }
        }
    }

    Dialog{
        id: simpleDialog
        acceptText: qsTr("Ok")
        headingText: qsTr("Package removed")

        inline: false

        icon: "image://theme/exclamation-triangle"

        onAccepted: {
            back();
        }
    }


    Dialog{
        id: errorDialog
        acceptText: qsTr("Ok")
        headingText: qsTr("Error!")
        subLabelText: qsTr("We have some error")

        inline: false

        icon: "image://theme/exclamation-triangle"

        onAccepted: {
            back();
        }
    }

    Connections{
        target: packageManager
        onRemovePackagesStarted: {
            removeProgressBar.visible = true
            removeProgressBar.value = 0
            controlsArea.visible = false
        }

        onRemovePackagesProgress: {
            removeProgressBar.value = percentage
        }

        onRemovePackagesFinished: {
           simpleDialog.open();
        }

        onRemovePackagesFail: {
            errorDialog.subLabelText = errorMessage
            errorDialog.open();
        }
    }

    function back(){
        if(action === "remove"){
            Qt.quit()
        } else {
            pageStack.pop()
        }
    }
}
