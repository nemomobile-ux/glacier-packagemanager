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

#include "packagemanagerdbus.h"
#include "database.h"

#include <QDBusConnection>
#include <QDBusError>
#include <QDebug>

PackageManagerDBus::PackageManagerDBus(DataBase* database, QObject* parent)
    : QObject(parent)
    , m_database(database)
{
    if (!m_database) {
        return;
    }

    connect(m_database, &DataBase::transactionStarted, this, &PackageManagerDBus::TransactionStarted);
    connect(m_database, &DataBase::transactionFinished, this, &PackageManagerDBus::TransactionFinished);
    connect(m_database, &DataBase::emitActionProgress, this, &PackageManagerDBus::ActionProgress);

    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.registerService(QStringLiteral("org.nemomobile.glacier.packagemanager"))) {
        qWarning() << "Failed to register D-Bus service:" << bus.lastError().message();
    }
    if (!bus.registerObject(QStringLiteral("/org/nemomobile/glacier/packagemanager"), this, QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals)) {
        qWarning() << "Failed to register D-Bus object:" << bus.lastError().message();
    }
}

void PackageManagerDBus::Install(const QString& packageName, const QString& repo)
{
    if (m_database) {
        m_database->install(packageName, repo);
    }
}

void PackageManagerDBus::Remove(const QString& packageName, const QString& repo)
{
    if (m_database) {
        m_database->remove(packageName, repo);
    }
}

void PackageManagerDBus::Upgrade(const QString& packageName, const QString& repo)
{
    if (m_database) {
        m_database->upgrade(packageName, repo);
    }
}

void PackageManagerDBus::Refresh()
{
    if (m_database) {
        m_database->refresh();
    }
}

void PackageManagerDBus::ShowUpdates()
{
    emit ShowUpdatesRequested();
}
