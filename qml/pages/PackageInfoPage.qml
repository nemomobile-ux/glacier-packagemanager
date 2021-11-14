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

Page {
    id: packageInfoPage

    property var item

    headerTools: HeaderToolsLayout {
        id: tools
        title: item.name
        showBackButton: true

        tools: [
            ToolButton {
                id: removeButton
                iconSource: "image://theme/trash"
                visible: item.installed
                onClicked: pkgTa.remove(item.name)
            },
            ToolButton {
                id: updateButton
                iconSource: "image://theme/refresh"
                visible: item.haveUpdates
            },
            ToolButton {
                id: installButton
                iconSource: "image://theme/download"
                visible: !item.installed
                onClicked: pkgTa.install(item.name)
            }
        ]
    }

    Item{
        id: rootView
        anchors.fill: parent

        Flickable{
            width: parent.width - Theme.itemSpacingLarge*2
            height: parent.height - Theme.itemSpacingLarge*2
            contentHeight: rootColumn.height

            anchors{
                top: parent.top
                topMargin: Theme.itemSpacingLarge
                left: parent.left
                leftMargin: Theme.itemSpacingLarge
            }

            Column{
                id: rootColumn
                spacing:Theme.itemSpacingLarge
                width: parent.width
                height: childrenRect.height

                Row {
                    id: appInfoRow
                    width: parent.width
                    height: childrenRect.height

                    spacing:Theme.itemSpacingLarge

                    Image{
                        width: height
                        height: appName.height
                        source: item.icon != "" ? item.icon : "/usr/share/glacier-packagemanager/images/glacier-packagemanager.png"
                    }

                    Label{
                        id: appName
                        width: parent.width
                        text: item.name
                        wrapMode: Text.WordWrap
                    }
                }

                Label{
                    width: parent.width
                    wrapMode: Text.WordWrap
                    text: item.description
                }

                ListView{
                    id: screenShotListview
                    width: parent.width
                    height: parent.width*3/4
                    model: item.screenshots
                    clip: true
                    visible: item.screenshots.length > 0
                    orientation: ListView.Horizontal
                    snapMode: ListView.SnapOneItem
                    highlightRangeMode: ListView.StrictlyEnforceRange
                    boundsBehavior: Flickable.StopAtBounds
                    delegate: Image {
                        width: screenShotListview.width
                        height: screenShotListview.height
                        source: modelData
                        fillMode: Image.PreserveAspectFit
                    }
                }

                Label{
                    width: parent.width
                    text: item.haveUpdates ? qsTr("New version") : qsTr("Version")
                    font.bold : true
                    font.pixelSize: Theme.fontSizeSmall
                }

                Label{
                    width: parent.width
                    text: item.version
                }

                Label{
                    width: parent.width
                    text: qsTr("Installed version")
                    font.bold : true
                    font.pixelSize: Theme.fontSizeSmall
                    visible: item.installed
                }

                Label{
                    width: parent.width
                    text: item.installedVersion
                    visible: item.installed
                }

                Label{
                    width: parent.width
                    text: qsTr("Repo")
                    font.bold : true
                    font.pixelSize: Theme.fontSizeSmall
                }

                Label{
                    width: parent.width
                    text: item.repo
                }
            }
        }
    }
}
