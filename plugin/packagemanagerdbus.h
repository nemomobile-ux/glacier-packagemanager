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

#ifndef PACKAGEMANAGERDBUS_H
#define PACKAGEMANAGERDBUS_H

#include <QObject>
#include <QString>

class DataBase;

class PackageManagerDBus : public QObject {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.nemomobile.glacier.packagemanager")

public:
    explicit PackageManagerDBus(DataBase* database, QObject* parent = nullptr);

public slots:
    void Install(const QString& packageName, const QString& repo);
    void Remove(const QString& packageName, const QString& repo);
    void Upgrade(const QString& packageName, const QString& repo);
    void Refresh();

signals:
    void TransactionStarted();
    void TransactionFinished();
    void ActionProgress(const QString& action, const QString& status, double progress);

private:
    DataBase* m_database = nullptr;
};

#endif // PACKAGEMANAGERDBUS_H
