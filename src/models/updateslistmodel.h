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

#ifndef UPDATESLISTMODEL_H
#define UPDATESLISTMODEL_H

#include <QObject>
#include <QAbstractListModel>

#include <XTransactionManager>

class UpdatesListModel : public QAbstractListModel
{
    Q_OBJECT

    struct UpdatesListItem{
        QString packageId;
        QString name;
        QString version;
        QString arch;
        QString repo;
        QString summary;
        bool selected;
    };

    enum UpdateItemRoles {
        PackageIdRole = Qt::UserRole + 1,
        NameRole,
        VersionRole,
        ArchRole,
        RepoRole,
        SummaryRole,
        SelectedRole
    };

public:
    explicit UpdatesListModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const {return m_hash;}

signals:
    void updatesReady();

public slots:
    void getUpdates();
    QStringList getSelectedPackagesIds();

private:
    QHash<int,QByteArray> m_hash;
    QVector<UpdatesListItem> m_updateList;

    void fillUpdatesList(XTransaction *transaction);
};

#endif // UPDATESLISTMODEL_H
