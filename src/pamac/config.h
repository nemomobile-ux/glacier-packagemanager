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

#ifndef CONFIG_H
#define CONFIG_H

#include <QObject>
#include <QString>
#include <QVariant>
#include "pamac.h"

class Config : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool recurse READ recurse WRITE setRecurse NOTIFY recurseChanged)
    Q_PROPERTY(bool noUpdateHideIcon READ noUpdateHideIcon WRITE setNoUpdateHideIcon NOTIFY noUpdateHideIconChanged)
    Q_PROPERTY(bool downloadUpdates READ downloadUpdates WRITE setDownloadUpdates NOTIFY downloadUpdatesChanged)
    Q_PROPERTY(bool cleanRmOnlyUninstalled READ cleanRmOnlyUninstalled WRITE setCleanRmOnlyUninstalled NOTIFY cleanRmOnlyUninstalledChanged)
    Q_PROPERTY(int maxParallelDownloads READ maxParallelDownloads WRITE setMaxParallelDownloads NOTIFY maxParallelDownloadsChanged)
    Q_PROPERTY(bool enableDowngrade READ enableDowngrade WRITE setEnableDowngrade NOTIFY enableDowngradeChanged)
    Q_PROPERTY(int refreshPeriod READ refreshPeriod WRITE setRefreshPeriod NOTIFY refreshPeriodChanged)
    Q_PROPERTY(int cleanKeepNumPkgs READ cleanKeepNumPkgs WRITE setCleanKeepNumPkgs NOTIFY cleanKeepNumPkgsChanged)
    Q_PROPERTY(bool checkspace READ checkspace WRITE setCheckspace NOTIFY checkspaceChanged)

public:
    Config(const QString& str,QObject* parent = nullptr);
    ~Config();

    PamacConfig* get() {return m_pmConfig;}
    Q_INVOKABLE QStringList getIgnorePkgs();
    Q_INVOKABLE void save();
    Q_INVOKABLE void reload();

    QString configPath() {return m_configPath;}
    bool recurse() {return m_recurse;}
    bool noUpdateHideIcon() {return m_noUpdateHideIcon;}
    bool downloadUpdates() {return m_downloadUpdates;}
    bool cleanRmOnlyUninstalled() {return m_cleanRmOnlyUninstalled;}
    int maxParallelDownloads() {return m_maxParallelDownloads;}
    bool enableDowngrade() {return m_enableDowngrade;}
    int refreshPeriod() {return m_refreshPeriod;}
    int cleanKeepNumPkgs() {return m_cleanKeepNumPkgs;}
    bool checkspace() {return m_checkspace;}

    void setRecurse(bool recurse);
    void setNoUpdateHideIcon(bool noUpdateHideIcon);
    void setDownloadUpdates(bool downloadUpdates);
    void setCleanRmOnlyUninstalled(bool cleanRmOnlyUninstalled);
    void setMaxParallelDownloads(int maxParallelDownloads);
    void setEnableDowngrade(bool enableDowngrade);
    void setRefreshPeriod(int refreshPeriod);
    void setCleanKeepNumPkgs(int cleanKeepNumPkgs);
    void setCheckspace(bool checkspace);

Q_SIGNALS:
    void recurseChanged();
    void noUpdateHideIconChanged();
    void downloadUpdatesChanged();
    void cleanRmOnlyUninstalledChanged();
    void maxParallelDownloadsChanged();
    void enableDowngradeChanged();
    void refreshPeriodChanged();
    void cleanKeepNumPkgsChanged();
    void checkspaceChanged();

private:
    PamacConfig* m_pmConfig;

    QString m_configPath;
    bool m_recurse;
    bool m_noUpdateHideIcon;
    bool m_downloadUpdates;
    bool m_cleanRmOnlyUninstalled;
    int m_maxParallelDownloads;
    bool m_enableDowngrade;
    int m_refreshPeriod;
    int m_cleanKeepNumPkgs;
    bool m_checkspace;
};

#endif // CONFIG_H
