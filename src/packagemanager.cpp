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

#include "packagemanager.h"

PackageManager::PackageManager(QObject *parent) : QObject(parent)
{

}


/**************** INSTALL ****************/
void PackageManager::installPackage(QStringList pkgList)
{
    if(pkgList.isEmpty()) {
        qWarning() << Q_FUNC_INFO << "Install list is empty";
        return;
    }

    XTransaction *transaction = XTransactionManager::resolve(pkgList, XTransactionNamespace::FilterNewest);
    connect(transaction, &XTransaction::finished, this, &PackageManager::_installPackage);
    transaction->start();
}

void PackageManager::_installPackage(XTransaction *taction)
{
    QStringList pkgIds;

    QVector<QVariantHash> results = taction->results();

    for (const QVariantHash &item : results) {
        pkgIds.append(item["packageID"].toString());
    }

    if(pkgIds.isEmpty())
    {
        qWarning() << Q_FUNC_INFO << "Install list is empty";
        return;
    }
    XTransaction *transaction = XTransactionManager::installPackages(pkgIds);
    connect(transaction, &XTransaction::failed, this, &PackageManager::onInstallPackagesFail);
    connect(transaction, &XTransaction::started, this, &PackageManager::installPackagesStarted);
    connect(transaction, &XTransaction::finished, this, &PackageManager::installPackagesFinished);
    connect(transaction, &XTransaction::progressChanged, this, &PackageManager::onInstallProgressChanged);

    transaction->start();
}

void PackageManager::onInstallPackagesFail(XTransaction *transaction)
{
    emit installPackagesFail(transaction->errorDetailsString());
}


void PackageManager::onInstallProgressChanged(XTransaction *transaction, uint percentage)
{
    Q_UNUSED(transaction);
    emit installPackagesProgress(percentage);
}


/**************** REMOVE ****************/
void PackageManager::removePackage(QStringList pkgList)
{
    if(pkgList.isEmpty()) {
        qWarning() << Q_FUNC_INFO << "Install list is empty";
        return;
    }

    XTransaction *transaction = XTransactionManager::resolve(pkgList, XTransactionNamespace::FilterInstalled);
    connect(transaction, &XTransaction::finished, this, &PackageManager::_removePackage);
    transaction->start();
}

void PackageManager::_removePackage(XTransaction *taction)
{
    QStringList pkgIds;

    QVector<QVariantHash> results = taction->results();

    for (const QVariantHash &item : results) {
        pkgIds.append(item["packageID"].toString());
    }

    if(pkgIds.isEmpty())
    {
        qWarning() << Q_FUNC_INFO << "Remove list is empty";
        return;
    }

    XTransaction *transaction = XTransactionManager::removePackages(pkgIds);
    connect(transaction, &XTransaction::failed, this, &PackageManager::onRemovePackagesFail);
    connect(transaction, &XTransaction::started, this, &PackageManager::removePackagesStarted);
    connect(transaction, &XTransaction::finished, this, &PackageManager::removePackagesFinished);
    connect(transaction, &XTransaction::progressChanged, this, &PackageManager::onRemoveProgressChanged);

    transaction->start();
}

void PackageManager::onRemovePackagesFail(XTransaction *transaction)
{
    emit removePackagesFail(transaction->errorDetailsString());
}

void PackageManager::onRemoveProgressChanged(XTransaction *transaction, uint percentage)
{
    Q_UNUSED(transaction);
    emit removePackagesProgress(percentage);
}

/**************** UPDATES ****************/
void PackageManager::installUpdates(QStringList pkgList)
{
    if(pkgList.count() == 0 )
    {
        qWarning() << Q_FUNC_INFO << "Updates list is empty";
        return;
    }

    XTransaction *transaction = XTransactionManager::updatePackages(pkgList);

    transaction->start();
}

void PackageManager::getUpdates()
{
    XTransaction *transaction = XTransactionManager::getUpdates();
    connect(transaction, &XTransaction::finished, this, &PackageManager::listOfUpdatesReady);

    transaction->start();
}

void PackageManager::refreshRepos(bool force)
{
    XTransaction *transaction = XTransactionManager::refreshCache(force);
    connect(transaction, &XTransaction::started, this, &PackageManager::refreshReposStarted);
    connect(transaction, &XTransaction::finished, this, &PackageManager::onRefreshFinished);
    connect(transaction, &XTransaction::progressChanged,this, &PackageManager::onRefreshProgressChanged);
    transaction->start();
}

void PackageManager::listOfUpdatesReady(XTransaction *transaction)
{
    QList<QVariantMap> updateList;

    QVector<QVariantHash> results = transaction->results();
    for (int i = 0; i < results.size(); ++i) {
        QString packageId = results[i]["packageID"].toString();
        QString name = XTransactionManager::packageName(packageId);
        QString version = XTransactionManager::packageVersion(packageId);
        QString arch = XTransactionManager::packageArch(packageId);
        QString repo = XTransactionManager::packageData(packageId);

        QString summary = results[i]["summary"].toString();

        updateList.append({{ QStringLiteral("name"), name },
                           { QStringLiteral("version"), version },
                           { QStringLiteral("arch"), arch },
                           { QStringLiteral("repo"), repo },
                           { QStringLiteral("summary"), summary}});
    }

    qDebug() << Q_FUNC_INFO << "Aviable " << updateList.size() << " updates";

    emit updatesReady();
}


void PackageManager::onRefreshFinished(XTransaction *transaction)
{
    emit refreshReposFinished(transaction->isSucceeded());
}

void PackageManager::onRefreshProgressChanged(XTransaction *transaction, uint percentage)
{
    Q_UNUSED(transaction);
    emit refreshReposProgress(percentage);
}

