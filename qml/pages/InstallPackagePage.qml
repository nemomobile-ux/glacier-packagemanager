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
    id: installPage

    headerTools: HeaderToolsLayout {
        id: tools
        title: qsTr("Install package")
        showBackButton: action !== "install"
    }

    Rectangle{
        id: removeArea

        width: parent.width
        height: parent.height

        color: "transparent"

        Label{
            id: installLabel
            text: qsTr("Install package") + " " + pkgname + "?";
            anchors.centerIn: parent
        }


        ProgressBar{
            id: installProgressBar

            width: installPage.width - Theme.itemSpacingLarge*2
            anchors{
                left: parent.left
                leftMargin: Theme.itemSpacingLarge
                top: installLabel.bottom
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
                text: qsTr("Install")

                anchors.bottom: parent.bottom

                onClicked: {
                    installProgressBar.visible = true
                    installProgressBar.value = 0
                    controlsArea.visible = false
                    installLabel.text =  qsTr("Installing package") + " " + pkgname;
                    packageManager.installPackage([pkgname])
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
                    back()
                }
            }
        }
    }

    Dialog{
        id: simpleDialog
        acceptText: qsTr("Ok")
        headingText: qsTr("Package installed")

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
        onInstallPackagesStarted: {
            installProgressBar.visible = true
            installProgressBar.value = 0
            controlsArea.visible = false
        }

        onInstallPackagesProgress: {
            installProgressBar.value = percentage
        }

        onInstallPackagesFinished: {
            simpleDialog.open();
        }

        onInstallPackagesFail: {
            errorDialog.subLabelText = errorMessage
            errorDialog.open();
        }
    }

    function back(){
        if(action === "install"){
            Qt.quit()
        } else {
            pageStack.pop()
        }
    }
}
