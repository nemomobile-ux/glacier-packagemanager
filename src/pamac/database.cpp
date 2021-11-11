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

#include "database.h"
#include "updates.h"
#include <QDateTime>
#include <QDebug>
#include <QThread>

DataBase::DataBase(QObject *parent) : QObject(parent)
{
    m_config = new Config("/etc/pamac.conf");
    m_pmDatabase = pamac_database_new(m_config->get());

    pamac_database_enable_appstream(m_pmDatabase);

    m_config->getIgnorePkgs();
}

void DataBase::searchPackages(const QString &name)
{
    m_results.clear();
    m_serching = 0;
    pamac_database_search_pkgs_async(m_pmDatabase, name.toUtf8(), searchFinish, this);
    pamac_database_search_installed_pkgs_async(m_pmDatabase, name.toUtf8(), searchFinish, this);
}

void DataBase::getUpdates()
{
    pamac_database_get_updates_async(m_pmDatabase, false, getUpdatesFinish, this);
}

QVariantMap DataBase::getPackage(const QString &name)
{
    PamacAlpmPackage *pkg = pamac_database_get_pkg(m_pmDatabase, name.toUtf8());
    if(pkg != nullptr) {
        return getPkg(&pkg->parent_instance);
    } else {
        qDebug() << "Not found";
    }
    return QVariantMap();
}

QStringList DataBase::getRepos()
{
    QStringList repos;
    GPtrArray *repoList = pamac_database_get_repos_names(m_pmDatabase);
    for(int i = 0; i < repoList->len; i++) {
        repos << (gchar*)(g_ptr_array_index(repoList, i));
    }
    return repos;
}

QStringList DataBase::getGroups()
{
    QStringList groups;
    GPtrArray *groupsList = pamac_database_get_groups_names(m_pmDatabase);
    for(int i = 0; i < groupsList->len; i++) {
        groups << (gchar*)(g_ptr_array_index(groupsList, i));
    }
    return groups;
}

void DataBase::getCategoryPackages(const QString &category)
{
    pamac_database_get_category_pkgs_async(m_pmDatabase,category.toUtf8(), getCategoryPackagesFinish ,this);
}

void DataBase::getGroupPackages(const QString &group)
{
    pamac_database_get_group_pkgs_async(m_pmDatabase, group.toUtf8(), getGroupPackagesFinish, this);
}

void DataBase::getRepoPackages(const QString &repo)
{
    pamac_database_get_repo_pkgs_async(m_pmDatabase, repo.toUtf8(), getRepoPackagesFinish, this);
}

void DataBase::getInstalledApps()
{
    pamac_database_get_installed_apps_async(m_pmDatabase, getInstalledAppsFinish, this);
}

QVariantMap DataBase::getPkg(PamacPackage *p)
{
    QString name = pamac_package_get_name(p);
    QString id = pamac_package_get_id(p);
    QString appName = pamac_package_get_app_name(p);
    QString appId = pamac_package_get_app_id(p);
    QString version = pamac_package_get_version(p);
    QString installedVersion = pamac_package_get_installed_version(p);
    QString description = pamac_package_get_desc(p);
    QString descriptionLong = pamac_package_get_long_desc(p);
    QString repo = pamac_package_get_repo(p);
    QString launchable = pamac_package_get_launchable(p);
    QString license = pamac_package_get_license(p);
    QString url = pamac_package_get_url(p);
    QString icon = pamac_package_get_icon(p);
    qint32 installedSize = pamac_package_get_installed_size(p);
    qint32 downloadedSize = pamac_package_get_download_size(p);

    QDateTime installdate = QDateTime::fromSecsSinceEpoch(0);
    GDateTime* inTime = pamac_package_get_install_date(p);
    if(inTime != nullptr) {
        installdate = QDateTime::fromSecsSinceEpoch(g_date_time_to_unix(inTime));
    }

    QStringList screenShotLists;
    GPtrArray* screenShots = pamac_package_get_screenshots(p);
    for(int i = 0; i < screenShots->len; i++) {
        screenShotLists << (gchar*)(g_ptr_array_index(screenShots, i));
    }


    return QVariantMap({{ QStringLiteral("name"), name },
                        { QStringLiteral("id"), id },
                        { QStringLiteral("appName"), appName },
                        { QStringLiteral("appId"), appId },
                        { QStringLiteral("version"), version },
                        { QStringLiteral("installedVersion"), installedVersion },
                        { QStringLiteral("installedTime"), installedSize },
                        { QStringLiteral("description"), description },
                        { QStringLiteral("descriptionLong"), descriptionLong },
                        { QStringLiteral("repo"), repo },
                        { QStringLiteral("launchable"), launchable },
                        { QStringLiteral("license"), license },
                        { QStringLiteral("url"), url },
                        { QStringLiteral("icon"), icon },
                        { QStringLiteral("installedSize"), installedSize },
                        { QStringLiteral("downloadedSize"), downloadedSize },
                        { QStringLiteral("screenshots"), screenShotLists },
                        { QStringLiteral("haveUpdates"), installedVersion != version && installedVersion != "" },
                        { QStringLiteral("installed"), installedVersion != ""}});

}

QList<QVariantMap> DataBase::gptrToPackageList(GPtrArray *pkgList)
{
    QList<QVariantMap> packages;
    if(pkgList == nullptr) {
        qWarning() << "Got nullptr";
        return packages;
    }

    for(int i = 0; i < pkgList->len; i++) {
        PamacPackage *p = static_cast<PamacPackage *>(g_ptr_array_index(pkgList, i));
        packages << getPkg(p);
    }
    return packages;
}

void DataBase::searchFinish(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
    DataBase *db = static_cast<DataBase*>(user_data);
    db->m_serching++;

    GPtrArray *pkgList = pamac_database_search_installed_pkgs_finish(db->m_pmDatabase, res);
    for(int i = 0; i < pkgList->len; i++) {
        PamacPackage *p = static_cast<PamacPackage *>(g_ptr_array_index(pkgList, i));
        QVariantMap pkg = db->getPkg(p);
        db->m_results.append(pkg);
    }

    if(db->m_serching >= 2) {
        Q_EMIT db->searchPackagesReady(db->m_results);
    }
}

void DataBase::getCategoryPackagesFinish(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
    qDebug() << Q_FUNC_INFO;

    DataBase *db = static_cast<DataBase*>(user_data);
    QList<QVariantMap> packages = db->gptrToPackageList(pamac_database_get_category_pkgs_finish(db->m_pmDatabase, res));
    Q_EMIT db->categoryPackagesReady(packages);
}

void DataBase::getGroupPackagesFinish(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
    DataBase *db = static_cast<DataBase*>(user_data);
    QList<QVariantMap> packages = db->gptrToPackageList(pamac_database_get_group_pkgs_finish(db->m_pmDatabase, res));
    Q_EMIT db->getGroupPackagesReady(packages);
}

void DataBase::getRepoPackagesFinish(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
    DataBase *db = static_cast<DataBase*>(user_data);
    QList<QVariantMap> packages = db->gptrToPackageList(pamac_database_get_repo_pkgs_finish(db->m_pmDatabase, res));
    Q_EMIT db->getRepoPackagesReady(packages);
}

void DataBase::getInstalledAppsFinish(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
    DataBase *db = static_cast<DataBase*>(user_data);
    QList<QVariantMap> packages = db->gptrToPackageList(pamac_database_get_installed_pkgs_finish(db->m_pmDatabase, res));
    Q_EMIT db->getInstalledAppsReady(packages);
}

void DataBase::getUpdatesFinish(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
    DataBase *db = static_cast<DataBase*>(user_data);
    Updates upd = pamac_database_get_updates_finish(db->m_pmDatabase,res);

    QList<QVariantMap> packages = db->gptrToPackageList(upd.get());
    Q_EMIT db->getUpdatesReady(packages);
}
