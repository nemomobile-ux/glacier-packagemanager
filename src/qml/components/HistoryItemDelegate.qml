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


ListViewItemWithActions {
    id: delegate

    property string pkgName
    property string pkgAction
    property string pkgVersion
    property string pkgDate

    label: pkgAction+" "+pkgName
    description: pkgVersion + " " + refreshTimestamp()
    showNext: false
    icon: calcIcon()


    function refreshTimestamp() {
        var timeAgo;
        var seconds = Math.floor((new Date() - pkgDate) / 1000)
        var years = Math.floor(seconds / (365*24*60*60))
        var months = Math.floor(seconds / (30*24*60*60))
        var days = Math.floor(seconds / (24*60*60))
        var hours = Math.floor(seconds / (60*60))
        var minutes = Math.floor(seconds / 60)

        if (years >= 1) {
            timeAgo = qsTr("%n year(s) ago", "actions", years)
        }else if (months >= 1) {
            timeAgo = qsTr("%n months(s) ago", "actions", months)
        }else if (days >= 1) {
            timeAgo = qsTr("%n day(s) ago", "actions", days)
        }else if (hours >= 1) {
            timeAgo = qsTr("%n hours(s) ago", "actions", hours)
        } else if (minutes >= 1) {
            timeAgo = qsTr("%n minutes(s) ago", "actions", minutes)
        } else {
            timeAgo = qsTr("Just now")
        }

        return timeAgo
    }

    function calcIcon() {
        if(pkgAction == "Upgraded") {
            return "image://theme/refresh"
        }

        if(pkgAction == "Installed") {
            return "image://theme/download"
        }

        if(pkgAction == "Removed") {
            return "image://theme/trash"
        }

        return "image://theme/box"
    }
}
