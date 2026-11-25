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

#include "flatpakbackend.h"
#include "flatpakbackendworker.h"

FlatpakBackend::FlatpakBackend()
{
    m_worker = new FlatpakBackendWorker();
    m_worker->moveToThread(&m_thread);

    connect(this, &FlatpakBackend::requestSearchPackages, m_worker, &FlatpakBackendWorker::doSearchPackages, Qt::QueuedConnection);
    connect(this, &FlatpakBackend::requestGetUpdates, m_worker, &FlatpakBackendWorker::doGetUpdates, Qt::QueuedConnection);
    connect(this, &FlatpakBackend::requestGetOrphansPackages, m_worker, &FlatpakBackendWorker::doGetOrphansPackages, Qt::QueuedConnection);
    connect(this, &FlatpakBackend::requestGetPackage, m_worker, &FlatpakBackendWorker::doGetPackage, Qt::QueuedConnection);
    connect(this, &FlatpakBackend::requestListRepositories, m_worker, &FlatpakBackendWorker::doListRepositories, Qt::QueuedConnection);
    connect(this, &FlatpakBackend::requestInstall, m_worker, &FlatpakBackendWorker::doInstall, Qt::QueuedConnection);
    connect(this, &FlatpakBackend::requestInstallFromLocalFile, m_worker, &FlatpakBackendWorker::doInstallFromLocalFile, Qt::QueuedConnection);
    connect(this, &FlatpakBackend::requestRemove, m_worker, &FlatpakBackendWorker::doRemove, Qt::QueuedConnection);
    connect(this, &FlatpakBackend::requestUpgrade, m_worker, &FlatpakBackendWorker::doUpgrade, Qt::QueuedConnection);
    connect(this, &FlatpakBackend::requestUpdate, m_worker, &FlatpakBackendWorker::doUpdate, Qt::QueuedConnection);

    connect(m_worker, &FlatpakBackendWorker::dbRefreshed, this, &FlatpakBackend::dbRefreshed, Qt::QueuedConnection);
    connect(m_worker, &FlatpakBackendWorker::searchPackagesReady, this, &FlatpakBackend::searchPackagesReady, Qt::QueuedConnection);
    connect(m_worker, &FlatpakBackendWorker::getUpdatesReady, this, &FlatpakBackend::getUpdatesReady, Qt::QueuedConnection);
    connect(m_worker, &FlatpakBackendWorker::getOrphansPackagesReady, this, &FlatpakBackend::getOrphansPackagesReady, Qt::QueuedConnection);
    connect(m_worker, &FlatpakBackendWorker::getPackageReady, this, &FlatpakBackend::getPackageReady, Qt::QueuedConnection);
    connect(m_worker, &FlatpakBackendWorker::listRepositoriesReady, this, &FlatpakBackend::listRepositoriesReady, Qt::QueuedConnection);
    connect(m_worker, &FlatpakBackendWorker::emitAction, this, &FlatpakBackend::emitAction, Qt::QueuedConnection);
    connect(m_worker, &FlatpakBackendWorker::emitActionProgress, this, &FlatpakBackend::emitActionProgress, Qt::QueuedConnection);

    m_thread.start();
}

FlatpakBackend::~FlatpakBackend()
{
    m_thread.quit();
    m_thread.wait();
    delete m_worker;
    m_worker = nullptr;
}

QString FlatpakBackend::backendId() const
{
    return QStringLiteral("flatpak");
}

bool FlatpakBackend::enabled()
{
    return true;
}

void FlatpakBackend::searchPackages(const QString& name)
{
    emit requestSearchPackages(name);
}

void FlatpakBackend::getUpdates()
{
    emit requestGetUpdates();
}

void FlatpakBackend::getOrphansPackages()
{
    emit requestGetOrphansPackages();
}

void FlatpakBackend::getPackage(const QString& name)
{
    emit requestGetPackage(name);
}

void FlatpakBackend::listRepositories()
{
    emit requestListRepositories();
}

void FlatpakBackend::install(QString packageName)
{
    emit requestInstall(packageName);
}

void FlatpakBackend::installFromLocalFile(QString patch)
{
    emit requestInstallFromLocalFile(patch);
}

void FlatpakBackend::remove(QString packageName)
{
    emit requestRemove(packageName);
}

void FlatpakBackend::upgrade(QString packageName)
{
    emit requestUpgrade(packageName);
}

void FlatpakBackend::update()
{
    emit requestUpdate();
}
