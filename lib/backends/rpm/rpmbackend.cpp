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

#include "rpmbackend.h"
#include "rpmbackendworker.h"

RpmBackend::RpmBackend()
{
    m_worker = new RpmBackendWorker();
    m_worker->moveToThread(&m_thread);

    connect(this, &RpmBackend::requestSearchPackages, m_worker, &RpmBackendWorker::doSearchPackages, Qt::QueuedConnection);
    connect(this, &RpmBackend::requestGetUpdates, m_worker, &RpmBackendWorker::doGetUpdates, Qt::QueuedConnection);
    connect(this, &RpmBackend::requestGetOrphansPackages, m_worker, &RpmBackendWorker::doGetOrphansPackages, Qt::QueuedConnection);
    connect(this, &RpmBackend::requestGetPackage, m_worker, &RpmBackendWorker::doGetPackage, Qt::QueuedConnection);
    connect(this, &RpmBackend::requestListRepositories, m_worker, &RpmBackendWorker::doListRepositories, Qt::QueuedConnection);
    connect(this, &RpmBackend::requestInstall, m_worker, &RpmBackendWorker::doInstall, Qt::QueuedConnection);
    connect(this, &RpmBackend::requestInstallFromLocalFile, m_worker, &RpmBackendWorker::doInstallFromLocalFile, Qt::QueuedConnection);
    connect(this, &RpmBackend::requestRemove, m_worker, &RpmBackendWorker::doRemove, Qt::QueuedConnection);
    connect(this, &RpmBackend::requestUpgrade, m_worker, &RpmBackendWorker::doUpgrade, Qt::QueuedConnection);
    connect(this, &RpmBackend::requestUpdate, m_worker, &RpmBackendWorker::doUpdate, Qt::QueuedConnection);

    connect(m_worker, &RpmBackendWorker::dbRefreshed, this, &RpmBackend::dbRefreshed, Qt::QueuedConnection);
    connect(m_worker, &RpmBackendWorker::searchPackagesReady, this, &RpmBackend::searchPackagesReady, Qt::QueuedConnection);
    connect(m_worker, &RpmBackendWorker::getUpdatesReady, this, &RpmBackend::getUpdatesReady, Qt::QueuedConnection);
    connect(m_worker, &RpmBackendWorker::getOrphansPackagesReady, this, &RpmBackend::getOrphansPackagesReady, Qt::QueuedConnection);
    connect(m_worker, &RpmBackendWorker::getPackageReady, this, &RpmBackend::getPackageReady, Qt::QueuedConnection);
    connect(m_worker, &RpmBackendWorker::listRepositoriesReady, this, &RpmBackend::listRepositoriesReady, Qt::QueuedConnection);
    connect(m_worker, &RpmBackendWorker::emitAction, this, &RpmBackend::emitAction, Qt::QueuedConnection);
    connect(m_worker, &RpmBackendWorker::emitActionProgress, this, &RpmBackend::emitActionProgress, Qt::QueuedConnection);

    m_thread.start();
}

RpmBackend::~RpmBackend()
{
    m_thread.quit();
    m_thread.wait();
    delete m_worker;
    m_worker = nullptr;
}

QString RpmBackend::backendId() const
{
    return QStringLiteral("rpm");
}

bool RpmBackend::enabled()
{
    return true;
}

void RpmBackend::searchPackages(const QString& name)
{
    emit requestSearchPackages(name);
}

void RpmBackend::getUpdates()
{
    emit requestGetUpdates();
}

void RpmBackend::getOrphansPackages()
{
    emit requestGetOrphansPackages();
}

void RpmBackend::getPackage(const QString& name)
{
    emit requestGetPackage(name);
}

void RpmBackend::listRepositories()
{
    emit requestListRepositories();
}

void RpmBackend::install(QString packageName)
{
    emit requestInstall(packageName);
}

void RpmBackend::installFromLocalFile(QString patch)
{
    emit requestInstallFromLocalFile(patch);
}

void RpmBackend::remove(QString packageName)
{
    emit requestRemove(packageName);
}

void RpmBackend::upgrade(QString packageName)
{
    emit requestUpgrade(packageName);
}

void RpmBackend::update()
{
    emit requestUpdate();
}
