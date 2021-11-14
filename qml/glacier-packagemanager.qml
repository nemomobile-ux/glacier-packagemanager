/*
 * Copyright (C) 2018-2021 Chupligin Sergey <neochapay@gmail.com>
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

import "pages"

import org.glacier.packagemanager 1.0

ApplicationWindow {
    id: root

    initialPage: firstpage();

    PackageDatabase{
        id: pkgDb
    }

    PackageTransaction{
        id: pkgTa
    }

    function firstpage() {
        if(action === "remove") {
            return Qt.createComponent(Qt.resolvedUrl("/usr/share/glacier-packagemanager/qml/pages/RemovePackagePage.qml"))
        } else if (action === "install") {
            return Qt.createComponent(Qt.resolvedUrl("/usr/share/glacier-packagemanager/qml/pages/InstallPackagePage.qml"))
        }
        return Qt.createComponent(Qt.resolvedUrl("/usr/share/glacier-packagemanager/qml/pages/MainPage.qml"))
    }
}
