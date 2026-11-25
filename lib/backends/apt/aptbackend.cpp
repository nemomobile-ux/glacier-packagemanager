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

#include "aptbackend.h"
#include "aptbackendworker.h"

AptBackend::AptBackend()
{
    m_worker = new AptBackendWorker();
    m_worker->moveToThread(&m_thread);

    connect(this, &AptBackend::requestSearchPackages, m_worker, &AptBackendWorker::doSearchPackages, Qt::QueuedConnection);
    connect(this, &AptBackend::requestGetUpdates, m_worker, &AptBackendWorker::doGetUpdates, Qt::QueuedConnection);
    connect(this, &AptBackend::requestGetOrphansPackages, m_worker, &AptBackendWorker::doGetOrphansPackages, Qt::QueuedConnection);
    connect(this, &AptBackend::requestGetPackage, m_worker, &AptBackendWorker::doGetPackage, Qt::QueuedConnection);
    connect(this, &AptBackend::requestInstall, m_worker, &AptBackendWorker::doInstall, Qt::QueuedConnection);
    connect(this, &AptBackend::requestInstallFromLocalFile, m_worker, &AptBackendWorker::doInstallFromLocalFile, Qt::QueuedConnection);
    connect(this, &AptBackend::requestRemove, m_worker, &AptBackendWorker::doRemove, Qt::QueuedConnection);
    connect(this, &AptBackend::requestUpgrade, m_worker, &AptBackendWorker::doUpgrade, Qt::QueuedConnection);
    connect(this, &AptBackend::requestUpdate, m_worker, &AptBackendWorker::doUpdate, Qt::QueuedConnection);

    connect(m_worker, &AptBackendWorker::dbRefreshed, this, &AptBackend::dbRefreshed, Qt::QueuedConnection);
    connect(m_worker, &AptBackendWorker::searchPackagesReady, this, &AptBackend::searchPackagesReady, Qt::QueuedConnection);
    connect(m_worker, &AptBackendWorker::getUpdatesReady, this, &AptBackend::getUpdatesReady, Qt::QueuedConnection);
    connect(m_worker, &AptBackendWorker::getOrphansPackagesReady, this, &AptBackend::getOrphansPackagesReady, Qt::QueuedConnection);
    connect(m_worker, &AptBackendWorker::getPackageReady, this, &AptBackend::getPackageReady, Qt::QueuedConnection);
    connect(m_worker, &AptBackendWorker::emitAction, this, &AptBackend::emitAction, Qt::QueuedConnection);
    connect(m_worker, &AptBackendWorker::emitActionProgress, this, &AptBackend::emitActionProgress, Qt::QueuedConnection);

    m_thread.start();
}

AptBackend::~AptBackend()
{
    m_thread.quit();
    m_thread.wait();
    delete m_worker;
    m_worker = nullptr;
}

QString AptBackend::backendId() const
{
    return QStringLiteral("apt");
}

bool AptBackend::enabled()
{
    return true;
}

void AptBackend::searchPackages(const QString& name)
{
    emit requestSearchPackages(name);
}

void AptBackend::getUpdates()
{
    emit requestGetUpdates();
}

void AptBackend::getOrphansPackages()
{
    emit requestGetOrphansPackages();
}

void AptBackend::getPackage(const QString& name)
{
    emit requestGetPackage(name);
}

void AptBackend::install(QString packageName)
{
    emit requestInstall(packageName);
}

void AptBackend::installFromLocalFile(QString patch)
{
    emit requestInstallFromLocalFile(patch);
}

void AptBackend::remove(QString packageName)
{
    emit requestRemove(packageName);
}

void AptBackend::upgrade(QString packageName)
{
    emit requestUpgrade(packageName);
}

void AptBackend::update()
{
    emit requestUpdate();
}
