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
    id: mainPage
    property var searchPackages: []

    headerTools: HeaderToolsLayout {
        id: tools
        title: qsTr("Package manager working...")
    }

    Column {
        anchors.centerIn: parent

        spacing:Theme.itemSpacingLarge
        width: parent.width
        height: childrenRect.height

        anchors{
            top: parent.top
            topMargin: Theme.itemSpacingLarge
            left: parent.left
            leftMargin: Theme.itemSpacingLarge
        }

        Label{
            id: emitActionLabel
            width: parent.width
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            text: qsTr("Package manager working...")
        }

        Label{
            id: statusLabel
            width: parent.width
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
        }

        ProgressBar {
            id: progressbar
            width: parent.width - Theme.itemSpacingLarge*2
            indeterminate: true
        }
    }

    Connections{
        target: pkgTa
        function onEmitActionProgress(emitAction, status, progress) {
            emitActionLabel.text = emitAction
            statusLabel.text = status
            progressbar.value = progress
            progressbar.indeterminate = false
        }

        function onEmitAction(emitAction) {
            emitActionLabel.text = emitAction
            statusLabel.text = ""
            progressbar.value = 0
            progressbar.indeterminate = true
        }

        function onStartWaiting() {
            emitActionLabel.text = qsTr("Waiting...")
            statusLabel.text = ""
            progressbar.value = 0
            progressbar.indeterminate = true
        }

        function onStartDownloading() {
            emitActionLabel.text = qsTr("Downloading...")
            statusLabel.text = ""
            progressbar.value = 0
            progressbar.indeterminate = true
        }

        function onStartPreparing() {
            emitActionLabel.text = qsTr("Preparing...")
            statusLabel.text = ""
            progressbar.value = 0
            progressbar.indeterminate = true
        }

        function onTransactionFinished() {
            pageStack.pop()
        }
    }
}
