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

#ifndef RPMBACKENDWORKER_H
#define RPMBACKENDWORKER_H

#include <QObject>
#include <QVariantList>
#include <glacierpackage.h>

class RpmBackendWorker : public QObject {
    Q_OBJECT
public:
    explicit RpmBackendWorker(QObject* parent = nullptr);

public slots:
    void doSearchPackages(const QString& name);
    void doGetUpdates();
    void doGetOrphansPackages();
    void doGetPackage(const QString& name);
    void doListRepositories();
    void doInstall(const QString& packageName);
    void doInstallFromLocalFile(const QString& patch);
    void doRemove(const QString& packageName);
    void doUpgrade(const QString& packageName);
    void doUpdate();

signals:
    void dbRefreshed();
    void searchPackagesReady(QVariantList packages);
    void getUpdatesReady(QVariantList packages);
    void getOrphansPackagesReady(QVariantList packages);
    void getPackageReady(GlacierPackage package);
    void listRepositoriesReady(QVariantList repositories);
    void emitAction(const QString& action);
    void emitActionProgress(const QString& action, const QString& status, double progress);

private:
    bool runHelper(const QStringList& args, const QString& actionLabel);
    bool ensureZypp(bool loadRepos);
    bool m_zyppInitialized = false;
    bool m_reposLoaded = false;
};

#endif // RPMBACKENDWORKER_H
