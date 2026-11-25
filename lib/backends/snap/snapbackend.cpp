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

#include "snapbackend.h"
#include "snapbackendworker.h"

SnapBackend::SnapBackend()
{
    m_worker = new SnapBackendWorker();
    m_worker->moveToThread(&m_thread);

    connect(this, &SnapBackend::requestSearchPackages, m_worker, &SnapBackendWorker::doSearchPackages, Qt::QueuedConnection);
    connect(this, &SnapBackend::requestGetUpdates, m_worker, &SnapBackendWorker::doGetUpdates, Qt::QueuedConnection);
    connect(this, &SnapBackend::requestGetOrphansPackages, m_worker, &SnapBackendWorker::doGetOrphansPackages, Qt::QueuedConnection);
    connect(this, &SnapBackend::requestGetPackage, m_worker, &SnapBackendWorker::doGetPackage, Qt::QueuedConnection);
    connect(this, &SnapBackend::requestInstall, m_worker, &SnapBackendWorker::doInstall, Qt::QueuedConnection);
    connect(this, &SnapBackend::requestInstallFromLocalFile, m_worker, &SnapBackendWorker::doInstallFromLocalFile, Qt::QueuedConnection);
    connect(this, &SnapBackend::requestRemove, m_worker, &SnapBackendWorker::doRemove, Qt::QueuedConnection);
    connect(this, &SnapBackend::requestUpgrade, m_worker, &SnapBackendWorker::doUpgrade, Qt::QueuedConnection);
    connect(this, &SnapBackend::requestUpdate, m_worker, &SnapBackendWorker::doUpdate, Qt::QueuedConnection);

    connect(m_worker, &SnapBackendWorker::dbRefreshed, this, &SnapBackend::dbRefreshed, Qt::QueuedConnection);
    connect(m_worker, &SnapBackendWorker::searchPackagesReady, this, &SnapBackend::searchPackagesReady, Qt::QueuedConnection);
    connect(m_worker, &SnapBackendWorker::getUpdatesReady, this, &SnapBackend::getUpdatesReady, Qt::QueuedConnection);
    connect(m_worker, &SnapBackendWorker::getOrphansPackagesReady, this, &SnapBackend::getOrphansPackagesReady, Qt::QueuedConnection);
    connect(m_worker, &SnapBackendWorker::getPackageReady, this, &SnapBackend::getPackageReady, Qt::QueuedConnection);
    connect(m_worker, &SnapBackendWorker::emitAction, this, &SnapBackend::emitAction, Qt::QueuedConnection);
    connect(m_worker, &SnapBackendWorker::emitActionProgress, this, &SnapBackend::emitActionProgress, Qt::QueuedConnection);

    m_thread.start();
}

SnapBackend::~SnapBackend()
{
    m_thread.quit();
    m_thread.wait();
    delete m_worker;
    m_worker = nullptr;
}

QString SnapBackend::backendId() const
{
    return QStringLiteral("snap");
}

bool SnapBackend::enabled()
{
    return true;
}

void SnapBackend::searchPackages(const QString& name)
{
    emit requestSearchPackages(name);
}

void SnapBackend::getUpdates()
{
    emit requestGetUpdates();
}

void SnapBackend::getOrphansPackages()
{
    emit requestGetOrphansPackages();
}

void SnapBackend::getPackage(const QString& name)
{
    emit requestGetPackage(name);
}

void SnapBackend::install(QString packageName)
{
    emit requestInstall(packageName);
}

void SnapBackend::installFromLocalFile(QString patch)
{
    emit requestInstallFromLocalFile(patch);
}

void SnapBackend::remove(QString packageName)
{
    emit requestRemove(packageName);
}

void SnapBackend::upgrade(QString packageName)
{
    emit requestUpgrade(packageName);
}

void SnapBackend::update()
{
    emit requestUpdate();
}
