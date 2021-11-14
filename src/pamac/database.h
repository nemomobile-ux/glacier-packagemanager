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

#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <pamac.h>

#include "config.h"
#include "updates.h"

class DataBase : public QObject
{
    Q_OBJECT
public:
    explicit DataBase(QObject *parent = nullptr);

    QVariantMap getPackage(const QString &name);

    Q_INVOKABLE QStringList getRepos();
    Q_INVOKABLE QStringList getGroups();
    Q_INVOKABLE void getCategoryPackages(const QString &category);
    Q_INVOKABLE void getGroupPackages(const QString &group);
    Q_INVOKABLE void getRepoPackages(const QString &repo);
    Q_INVOKABLE void getInstalledApps();

    Q_INVOKABLE void searchPackages(const QString &name);

    Q_INVOKABLE void getUpdates();
    Q_INVOKABLE void refresh();

    PamacDatabase* db() {return m_pmDatabase;}
    Config* config() {return m_config;}

Q_SIGNALS:
    void categoryPackagesReady(QList<QVariantMap> packages);
    void searchPackagesReady(QList<QVariantMap> packages);
    void getGroupPackagesReady(QList<QVariantMap> packages);
    void getRepoPackagesReady(QList<QVariantMap> packages);
    void getInstalledAppsReady(QList<QVariantMap> packages);
    void getUpdatesReady(QList<QVariantMap> packages);
    void dbRefreshed();

private:
    PamacDatabase* m_pmDatabase;

    Config* m_config;
    int m_serching;

    QList<QVariantMap> gptrToPackageList(GPtrArray *pkgList);
    QVariantMap getPkg(PamacPackage *p);
    QVariantMap getPkg(PamacAlpmPackage *p);

    QList<QVariantMap> m_results;

    static void searchFinish(GObject *source_object, GAsyncResult *res, gpointer user_data);
    static void getCategoryPackagesFinish(GObject *source_object, GAsyncResult *res, gpointer user_data);
    static void getGroupPackagesFinish(GObject *source_object, GAsyncResult *res, gpointer user_data);
    static void getRepoPackagesFinish(GObject *source_object, GAsyncResult *res, gpointer user_data);
    static void getInstalledAppsFinish(GObject *source_object, GAsyncResult *res, gpointer user_data);
    static void getUpdatesFinish(GObject *source_object, GAsyncResult *res, gpointer user_data);
};

#endif // DATABASE_H
