/*
 * Copyright (C) 2025 Chupligin Sergey <neochapay@gmail.com>
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

#include "database.h"
#include "glacierpmmanager.h"

#include <QDebug>

DataBase::DataBase(QObject* parent)
    : QObject { parent }
{
    qRegisterMetaType<GlacierPackage>("GlacierPackage");
    GlacierPMManager* manager = new GlacierPMManager(this);
    m_backend = manager->getBackend();
    if (!m_backend) {
        qWarning() << "Cannot load package manager backend";
        return;
    }

    connect(m_backend, &GlacierPMBackend::dbRefreshed, this, [this]() {
        emit dbRefreshed();
        if (m_transactionActive) {
            m_transactionActive = false;
            emit transactionFinished();
        }
    });
    connect(m_backend, &GlacierPMBackend::searchPackagesReady, this, &DataBase::searchPackagesReady);
    connect(m_backend, &GlacierPMBackend::getUpdatesReady, this, &DataBase::getUpdatesReady);
    connect(m_backend, &GlacierPMBackend::getOrphansPackagesReady, this, &DataBase::getOrphansPackagesReady);
    connect(m_backend, &GlacierPMBackend::getPackageReady, this, &DataBase::getPackageReady);
    connect(m_backend, &GlacierPMBackend::emitAction, this, &DataBase::emitAction);
    connect(m_backend, &GlacierPMBackend::emitActionProgress, this, &DataBase::emitActionProgress);
}

DataBase::~DataBase() = default;

void DataBase::searchPackages(const QString& name)
{
    if (!m_backend) {
        emit searchPackagesReady(QVariantList());
        return;
    }
    m_backend->searchPackages(name);
}

void DataBase::getUpdates()
{
    if (!m_backend) {
        emit getUpdatesReady(QVariantList());
        return;
    }
    m_backend->getUpdates();
}

void DataBase::getOrphansPackages()
{
    if (!m_backend) {
        emit getOrphansPackagesReady(QVariantList());
        return;
    }
    m_backend->getOrphansPackages();
}

void DataBase::getPackage(const QString& name)
{
    if (!m_backend) {
        emit getPackageReady(GlacierPackage());
        return;
    }
    m_backend->getPackage(name);
}

void DataBase::install(const QString& packageName)
{
    if (!m_backend) {
        return;
    }
    m_transactionActive = true;
    emit transactionStarted();
    emit emitAction(tr("Installing %1").arg(packageName));
    m_backend->install(packageName);
}

void DataBase::remove(const QString& packageName)
{
    if (!m_backend) {
        return;
    }
    m_transactionActive = true;
    emit transactionStarted();
    emit emitAction(tr("Removing %1").arg(packageName));
    m_backend->remove(packageName);
}

void DataBase::upgrade(const QString& packageName)
{
    if (!m_backend) {
        return;
    }
    m_transactionActive = true;
    emit transactionStarted();
    if (packageName.isEmpty()) {
        emit emitAction(tr("Upgrading all packages"));
    } else {
        emit emitAction(tr("Upgrading %1").arg(packageName));
    }
    m_backend->upgrade(packageName);
}

void DataBase::refresh()
{
    if (!m_backend) {
        return;
    }
    m_backend->update();
}
