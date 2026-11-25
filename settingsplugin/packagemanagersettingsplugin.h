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

#ifndef PACKAGEMANAGERSETTINGSPLUGIN_H
#define PACKAGEMANAGERSETTINGSPLUGIN_H

#include <glaciersettingsplugin.h>

class PackageManagerSettingsPlugin : public GlacierSettingsPlugin {
    Q_OBJECT
    Q_INTERFACES(GlacierSettingsPlugin)
    Q_PLUGIN_METADATA(IID "Glacier.SettingsPlugin")
public:
    explicit PackageManagerSettingsPlugin(QObject* parent = nullptr);
    PluginCategory category() const { return PluginCategory::Other; }
    QString id() const { return "packagemanager"; }
    QString title() const { return tr("Package manager"); }
    QString description() const { return tr("Repositories and update settings"); }
    QString qmlPath() const { return "/usr/share/glacier-settings/plugins/packagemanager/packagemanager.qml"; }
    QString icon() const { return "/usr/share/glacier-settings/plugins/packagemanager/packagemanager.svg"; }
    bool enabled() { return true; }
};

#endif // PACKAGEMANAGERSETTINGSPLUGIN_H
