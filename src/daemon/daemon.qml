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
import Nemo.Configuration
import Nemo.Notifications
import Connman

import Glacier.PackageManager

Item {
    id: root
    visible: false

    ConfigurationValue {
        id: lastCheckValue
        key: "/home/glacier/packagemanager/lastUpdateCheck"
        defaultValue: 0
    }

    ConfigurationValue {
        id: connectionPolicyValue
        key: "/home/glacier/packagemanager/updateConnectionPolicy"
        defaultValue: "wifiEthernet"
    }

    NetworkManager {
        id: networkManager
    }

    PackageDatabase {
        id: pkgDb
        onGetUpdatesReady: function(packages) {
            console.log("glacier-packagemanager-daemon: updates ready, count=" + packages.length)
            lastCheckValue.value = Date.now()
            if (packages.length > 0) {
                updateNotification.summary = qsTr("Updates available")
                updateNotification.body = qsTr("%n update(s) available", "", packages.length)
                updateNotification.itemCount = packages.length
                updateNotification.publish()
            }
            quitTimer.start()
        }
    }

    Notification {
        id: updateNotification
        category: "x-nemo.system-update"
        appName: qsTr("Package manager")
        itemCount: 0
        remoteActions: [{
            "name": "default",
            "service": "org.nemomobile.glacier.packagemanager",
            "path": "/org/nemomobile/glacier/packagemanager",
            "iface": "org.nemomobile.glacier.packagemanager",
            "method": "ShowUpdates"
        }]
    }

    property int networkRetries: 0

    Timer {
        id: quitTimer
        interval: 3000
        repeat: false
        onTriggered: Qt.quit()
    }

    Timer {
        id: startTimer
        interval: 2000
        repeat: false
        onTriggered: root.maybeCheck()
    }

    Timer {
        id: safetyQuitTimer
        interval: 120000
        repeat: false
        running: true
        onTriggered: {
            console.log("glacier-packagemanager-daemon: safety timeout, quitting")
            Qt.quit()
        }
    }

    function isAllowedConnection() {
        if (connectionPolicyValue.value === "any") {
            return networkManager.connected
        }
        return networkManager.connectedWifi !== null || networkManager.connectedEthernet !== null
    }

    function maybeCheck() {
        console.log("glacier-packagemanager-daemon: checking, valid=" + networkManager.valid
            + " connected=" + networkManager.connected
            + " wifi=" + (networkManager.connectedWifi !== null)
            + " ethernet=" + (networkManager.connectedEthernet !== null)
            + " policy=" + connectionPolicyValue.value)

        if (!networkManager.valid && networkRetries < 10) {
            networkRetries++
            startTimer.interval = 2000
            startTimer.restart()
            return
        }

        if (!isAllowedConnection()) {
            console.log("glacier-packagemanager-daemon: connection not allowed, quitting")
            Qt.quit()
            return
        }

        var now = Date.now()
        var last = Number(lastCheckValue.value)
        if (last > 0 && (now - last) < 24 * 60 * 60 * 1000) {
            console.log("glacier-packagemanager-daemon: checked recently, quitting")
            Qt.quit()
            return
        }

        console.log("glacier-packagemanager-daemon: requesting updates")
        pkgDb.getUpdates()
    }

    Component.onCompleted: {
        startTimer.start()
    }
}
