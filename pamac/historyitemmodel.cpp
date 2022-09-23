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

#include "historyitemmodel.h"
#include <QFile>

HistoryItemModel::HistoryItemModel(QObject* parent)
    : QAbstractListModel(parent)
{
    QFile file("/var/log/pacman.log");
    if (!file.exists()) {
        return;
    }

    file.open(QFile::ReadOnly);
    QStringList list;
    while (!file.atEnd()) {
        QString line = QString::fromUtf8(file.readLine());
        if (line.contains("[ALPM]")) {
            list.prepend(line);
        }
    }
    file.close();

    HistoryItem itm;
    m_historyList = itm.fromStringList(list);

    m_hash.insert(Qt::UserRole, QByteArray("name"));
    m_hash.insert(Qt::UserRole + 1, QByteArray("type"));
    m_hash.insert(Qt::UserRole + 2, QByteArray("date"));
    m_hash.insert(Qt::UserRole + 3, QByteArray("version"));
}

QList<HistoryItem> HistoryItemModel::historyList() const
{
    return m_historyList;
}

int HistoryItemModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return m_historyList.length();
}

int HistoryItemModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return m_hash.count();
}

QVariant HistoryItemModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (index.row() >= m_historyList.size())
        return QVariant();

    HistoryItem hItem = m_historyList.at(index.row());

    if (role == Qt::UserRole) {
        return hItem.name();
    }

    if (role == Qt::UserRole + 1) {
        return hItem.typeToString(hItem.type());
    }

    if (role == Qt::UserRole + 2) {
        return hItem.time().toMSecsSinceEpoch();
    }

    if (role == Qt::UserRole + 3) {
        return hItem.version();
    }

    return QVariant();
}
