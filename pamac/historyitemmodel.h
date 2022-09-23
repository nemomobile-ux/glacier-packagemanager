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

#ifndef HISTORYITEMMODEL_H
#define HISTORYITEMMODEL_H

#include <QAbstractListModel>
#include <QObject>
#include <QVariant>

#include "historyitem.h"

class HistoryItemModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(QList<HistoryItem> historyList READ historyList NOTIFY historyListChanged)
    Q_PROPERTY(int columnCount READ columnCount CONSTANT)
    Q_PROPERTY(int rowCount READ rowCount CONSTANT)
public:
    explicit HistoryItemModel(QObject* parent = nullptr);

    QList<HistoryItem> historyList() const;

public:
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override { return m_hash; }

Q_SIGNALS:
    void historyListChanged(const QList<HistoryItem>& historyList);

private:
    QHash<int, QByteArray> m_hash;
    QList<HistoryItem> m_historyList;
};

#endif // HISTORYITEMMODEL_H
