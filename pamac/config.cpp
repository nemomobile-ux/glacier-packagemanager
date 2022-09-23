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

#include "config.h"
#include <QDebug>

Config::Config(const QString& str, QObject* parent)
{
    m_pmConfig = pamac_config_new(str.toUtf8());
    m_configPath = str;

    m_recurse = pamac_config_get_recurse(m_pmConfig);
    m_noUpdateHideIcon = pamac_config_get_no_update_hide_icon(m_pmConfig);
    m_downloadUpdates = pamac_config_get_download_updates(m_pmConfig);
    m_cleanRmOnlyUninstalled = pamac_config_get_clean_rm_only_uninstalled(m_pmConfig);
    m_maxParallelDownloads = pamac_config_get_max_parallel_downloads(m_pmConfig);
    m_enableDowngrade = pamac_config_get_enable_downgrade(m_pmConfig);
    m_refreshPeriod = pamac_config_get_refresh_period(m_pmConfig);
    m_cleanKeepNumPkgs = pamac_config_get_clean_keep_num_pkgs(m_pmConfig);
    m_checkspace = pamac_config_get_checkspace(m_pmConfig);
}

Config::~Config()
{
    g_object_unref(m_pmConfig);
}

QStringList Config::getIgnorePkgs()
{
    QStringList packages;
    GList* ignoredPackages = g_hash_table_get_values(pamac_config_get_ignorepkgs(m_pmConfig));
    while (ignoredPackages) {
        GList* next = ignoredPackages->next;
        packages << (gchar*)next->data;
    }

    return packages;
}

void Config::save()
{
    pamac_config_save(m_pmConfig);
}

void Config::reload()
{
    pamac_config_reload(m_pmConfig);
}

void Config::addIgnorePkg(QString name)
{
    pamac_config_add_ignorepkg(m_pmConfig, name.toUtf8());
}

void Config::removeIgnorePkg(QString name)
{
    pamac_config_remove_ignorepkg(m_pmConfig, name.toUtf8());
}

void Config::setRecurse(bool recurse)
{
    if (recurse != m_recurse) {
        pamac_config_set_recurse(m_pmConfig, recurse);
        m_recurse = recurse;
        Q_EMIT recurseChanged();
    }
}

void Config::setNoUpdateHideIcon(bool noUpdateHideIcon)
{
    if (noUpdateHideIcon != m_noUpdateHideIcon) {
        pamac_config_set_no_update_hide_icon(m_pmConfig, noUpdateHideIcon);
        m_noUpdateHideIcon = noUpdateHideIcon;
        Q_EMIT noUpdateHideIconChanged();
    }
}

void Config::setDownloadUpdates(bool downloadUpdates)
{
    if (downloadUpdates != m_downloadUpdates) {
        pamac_config_set_download_updates(m_pmConfig, downloadUpdates);
        m_downloadUpdates = downloadUpdates;
        Q_EMIT downloadUpdatesChanged();
    }
}

void Config::setCleanRmOnlyUninstalled(bool cleanRmOnlyUninstalled)
{
    if (cleanRmOnlyUninstalled != m_cleanRmOnlyUninstalled) {
        pamac_config_set_clean_rm_only_uninstalled(m_pmConfig, cleanRmOnlyUninstalled);
        m_cleanRmOnlyUninstalled = cleanRmOnlyUninstalled;
        Q_EMIT cleanRmOnlyUninstalledChanged();
    }
}

void Config::setMaxParallelDownloads(int maxParallelDownloads)
{
    if (maxParallelDownloads != m_maxParallelDownloads) {
        pamac_config_set_max_parallel_downloads(m_pmConfig, maxParallelDownloads);
        m_maxParallelDownloads = maxParallelDownloads;
        Q_EMIT maxParallelDownloadsChanged();
    }
}

void Config::setEnableDowngrade(bool enableDowngrade)
{
    if (enableDowngrade != m_enableDowngrade) {
        pamac_config_set_enable_downgrade(m_pmConfig, enableDowngrade);
        m_enableDowngrade = enableDowngrade;
        Q_EMIT enableDowngradeChanged();
    }
}

void Config::setRefreshPeriod(int refreshPeriod)
{
    if (refreshPeriod != m_refreshPeriod) {
        pamac_config_set_refresh_period(m_pmConfig, refreshPeriod);
        m_refreshPeriod = refreshPeriod;
        Q_EMIT refreshPeriodChanged();
    }
}

void Config::setCleanKeepNumPkgs(int cleanKeepNumPkgs)
{
    if (cleanKeepNumPkgs != m_cleanKeepNumPkgs) {
        pamac_config_set_clean_keep_num_pkgs(m_pmConfig, cleanKeepNumPkgs);
        m_cleanKeepNumPkgs = cleanKeepNumPkgs;
        Q_EMIT cleanKeepNumPkgsChanged();
    }
}

void Config::setCheckspace(bool checkspace)
{
    if (checkspace != m_checkspace) {
        pamac_config_set_checkspace(m_pmConfig, checkspace);
        m_checkspace = checkspace;
        Q_EMIT checkspaceChanged();
    }
}
