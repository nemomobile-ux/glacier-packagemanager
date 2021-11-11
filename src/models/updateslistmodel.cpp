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

#include "updateslistmodel.h"

UpdatesListModel::UpdatesListModel(QObject *parent)
    : QAbstractListModel(parent)
{
    m_hash.insert(PackageIdRole ,QByteArray("packageID"));
    m_hash.insert(NameRole ,QByteArray("name"));
    m_hash.insert(VersionRole ,QByteArray("version"));
    m_hash.insert(ArchRole ,QByteArray("arch"));
    m_hash.insert(RepoRole ,QByteArray("repo"));
    m_hash.insert(SummaryRole ,QByteArray("summary"));
    m_hash.insert(SelectedRole ,QByteArray("selected"));
    getUpdates();
}

int UpdatesListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_updateList.count();
}

QVariant UpdatesListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= m_updateList.count())
        return QVariant();

    const UpdatesListItem item = m_updateList.at(index.row());

    switch (role) {
    case PackageIdRole:
        return item.packageId;
    case NameRole:
        return item.name;
    case VersionRole:
        return item.version;
    case ArchRole:
        return item.arch;
    case RepoRole:
        return item.repo;
    case SummaryRole:
        return item.summary;
    case SelectedRole:
        return item.selected;
    default:
        return QVariant();
    }
}

void UpdatesListModel::getUpdates()
{
    XTransaction *transaction = XTransactionManager::getUpdates();
    connect(transaction, &XTransaction::finished, this, &UpdatesListModel::fillUpdatesList);

    transaction->start();
}

QStringList UpdatesListModel::getSelectedPackagesIds()
{
    QStringList updPackages;
    for (const UpdatesListItem &item : m_updateList) {
        if(item.selected) {
            updPackages.append(item.packageId);
        }
    }
    return updPackages;
}

void UpdatesListModel::fillUpdatesList(XTransaction *transaction)
{
    QVector<QVariantHash> results = transaction->results();
    beginInsertRows(QModelIndex(),
                    m_updateList.count(),
                    m_updateList.count() + results.count() - 1);
    for (const QVariantHash &result : results) {

        UpdatesListItem item;

        item.packageId = result["packageID"].toString();
        item.name = XTransactionManager::packageName(item.packageId);
        item.version = XTransactionManager::packageVersion(item.packageId);
        item.arch = XTransactionManager::packageArch(item.packageId);
        item.repo = XTransactionManager::packageData(item.packageId);

        item.summary = result["summary"].toString();
//By default all package selected to update
        item.selected = true;

        m_updateList.append(item);

    }

    endInsertRows();

    qDebug() << Q_FUNC_INFO << "Available " << m_updateList.count() << " updates";
    Q_EMIT updatesReady();
}
