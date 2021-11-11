/*
 * Copyright (C) 2018 Chupligin Sergey <neochapay@gmail.com>
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

#ifndef PACKAGEMANAGER_H
#define PACKAGEMANAGER_H

#include <QObject>
#include <XTransactionManager>

class PackageManager : public QObject
{
    Q_OBJECT
public:
    explicit PackageManager(QObject *parent = nullptr);

Q_SIGNALS:
    void refreshReposStarted();
    void refreshReposFinished(bool success);
    void refreshReposProgress(uint percentage);

    void updatesReady();
    void updatePackagesFail(QString errorMessage);
    void updatePackagesFinished();
    void updatePackagesStarted();
    void updatePackagesProgress(uint percentage);

    void installPackagesSuccessed();
    void installPackagesFail(QString errorMessage);
    void installPackagesFinished();
    void installPackagesStarted();
    void installPackagesProgress(uint percentage);

    void removePackagesSuccessed();
    void removePackagesFail(QString errorMessage);
    void removePackagesFinished();
    void removePackagesStarted();
    void removePackagesProgress(uint percentage);

public Q_SLOTS:
    void refreshRepos(bool force = false);

    void getUpdates();
    void installUpdates(QStringList pkgList);

    void installPackage(QStringList pkgList);
    void removePackage(QStringList pkgList);

private Q_SLOTS:
    void onRefreshFinished(XTransaction *transaction);
    void onRefreshProgressChanged(XTransaction *transaction, uint percentage);

    void listOfUpdatesReady(XTransaction *transaction);
    void onUpdateProgressChanged(XTransaction *transaction, uint percentage);

    void _installPackage(XTransaction *taction);
    void onInstallPackagesFail(XTransaction *transaction);
    void onInstallProgressChanged(XTransaction *transaction, uint percentage);

    void _removePackage(XTransaction *taction);
    void onRemovePackagesFail(XTransaction *transaction);
    void onRemoveProgressChanged(XTransaction *transaction, uint percentage);
};

#endif // PACKAGEMANAGER_H
