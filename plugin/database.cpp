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
    m_backends = manager->backends();
    if (m_backends.isEmpty()) {
        qWarning() << "Cannot load package manager backend";
        return;
    }

    for (GlacierPMBackend* backend : m_backends) {
        connect(backend, &GlacierPMBackend::dbRefreshed, this, [this]() {
            emit dbRefreshed();
            if (m_transactionActiveCount > 0) {
                m_transactionActiveCount--;
                if (m_transactionActiveCount == 0) {
                    emit transactionFinished();
                }
            }
        });
        connect(backend, &GlacierPMBackend::emitAction, this, &DataBase::emitAction);
        connect(backend, &GlacierPMBackend::emitActionProgress, this, &DataBase::emitActionProgress);
    }
}

DataBase::~DataBase() = default;

GlacierPMBackend* DataBase::backendForRepo(const QString& repo) const
{
    if (repo.isEmpty()) {
        if (m_backends.size() == 1) {
            return m_backends.first();
        }
        return nullptr;
    }
    for (GlacierPMBackend* backend : m_backends) {
        if (backend->backendId() == repo) {
            return backend;
        }
    }
    return nullptr;
}

void DataBase::searchPackages(const QString& name)
{
    if (m_backends.isEmpty()) {
        emit searchPackagesReady(QVariantList());
        return;
    }
    const int gen = ++m_searchGeneration;
    m_searchPending = m_backends.size();
    m_searchResults.clear();
    for (GlacierPMBackend* backend : m_backends) {
        connect(backend, &GlacierPMBackend::searchPackagesReady, this, [this, gen](QVariantList packages) {
            if (gen != m_searchGeneration) {
                return;
            }
            m_searchResults.append(packages);
            m_searchPending--;
            if (m_searchPending <= 0) {
                emit searchPackagesReady(m_searchResults);
            } }, Qt::SingleShotConnection);
        backend->searchPackages(name);
    }
}

void DataBase::getUpdates()
{
    if (m_backends.isEmpty()) {
        emit getUpdatesReady(QVariantList());
        return;
    }
    const int gen = ++m_updatesGeneration;
    m_updatesPending = m_backends.size();
    m_updatesResults.clear();
    for (GlacierPMBackend* backend : m_backends) {
        connect(backend, &GlacierPMBackend::getUpdatesReady, this, [this, gen](QVariantList packages) {
            if (gen != m_updatesGeneration) {
                return;
            }
            m_updatesResults.append(packages);
            m_updatesPending--;
            if (m_updatesPending <= 0) {
                emit getUpdatesReady(m_updatesResults);
            } }, Qt::SingleShotConnection);
        backend->getUpdates();
    }
}

void DataBase::getOrphansPackages()
{
    if (m_backends.isEmpty()) {
        emit getOrphansPackagesReady(QVariantList());
        return;
    }
    const int gen = ++m_orphansGeneration;
    m_orphansPending = m_backends.size();
    m_orphansResults.clear();
    for (GlacierPMBackend* backend : m_backends) {
        connect(backend, &GlacierPMBackend::getOrphansPackagesReady, this, [this, gen](QVariantList packages) {
            if (gen != m_orphansGeneration) {
                return;
            }
            m_orphansResults.append(packages);
            m_orphansPending--;
            if (m_orphansPending <= 0) {
                emit getOrphansPackagesReady(m_orphansResults);
            } }, Qt::SingleShotConnection);
        backend->getOrphansPackages();
    }
}

void DataBase::getPackage(const QString& name, const QString& repo)
{
    if (m_backends.isEmpty()) {
        emit getPackageReady(GlacierPackage());
        return;
    }

    const int gen = ++m_packageGeneration;
    m_packageResult = GlacierPackage();
    m_packageFound = false;

    GlacierPMBackend* target = backendForRepo(repo);
    if (target) {
        m_packagePending = 1;
        connect(target, &GlacierPMBackend::getPackageReady, this, [this, gen](GlacierPackage package) {
            if (gen != m_packageGeneration) {
                return;
            }
            if (!package.name.isEmpty()) {
                m_packageResult = package;
                m_packageFound = true;
            }
            m_packagePending--;
            if (m_packagePending <= 0) {
                emit getPackageReady(m_packageResult);
            } }, Qt::SingleShotConnection);
        target->getPackage(name);
        return;
    }

    m_packagePending = m_backends.size();
    for (GlacierPMBackend* backend : m_backends) {
        connect(backend, &GlacierPMBackend::getPackageReady, this, [this, gen](GlacierPackage package) {
            if (gen != m_packageGeneration) {
                return;
            }
            if (!m_packageFound && !package.name.isEmpty()) {
                m_packageResult = package;
                m_packageFound = true;
            }
            m_packagePending--;
            if (m_packagePending <= 0) {
                emit getPackageReady(m_packageResult);
            } }, Qt::SingleShotConnection);
        backend->getPackage(name);
    }
}

void DataBase::install(const QString& packageName, const QString& repo)
{
    GlacierPMBackend* backend = backendForRepo(repo);
    if (!backend) {
        qWarning() << "No backend for install" << packageName << repo;
        return;
    }
    m_transactionActiveCount = 1;
    emit transactionStarted();
    emit emitAction(tr("Installing %1").arg(packageName));
    backend->install(packageName);
}

void DataBase::remove(const QString& packageName, const QString& repo)
{
    GlacierPMBackend* backend = backendForRepo(repo);
    if (!backend) {
        qWarning() << "No backend for remove" << packageName << repo;
        return;
    }
    m_transactionActiveCount = 1;
    emit transactionStarted();
    emit emitAction(tr("Removing %1").arg(packageName));
    backend->remove(packageName);
}

void DataBase::upgrade(const QString& packageName, const QString& repo)
{
    GlacierPMBackend* backend = backendForRepo(repo);
    if (!backend) {
        if (packageName.isEmpty() && !m_backends.isEmpty()) {
            m_transactionActiveCount = m_backends.size();
            emit transactionStarted();
            emit emitAction(tr("Upgrading all packages"));
            for (GlacierPMBackend* b : m_backends) {
                b->upgrade(QString());
            }
            return;
        }
        qWarning() << "No backend for upgrade" << packageName << repo;
        return;
    }
    m_transactionActiveCount = 1;
    emit transactionStarted();
    if (packageName.isEmpty()) {
        emit emitAction(tr("Upgrading all packages"));
    } else {
        emit emitAction(tr("Upgrading %1").arg(packageName));
    }
    backend->upgrade(packageName);
}

void DataBase::refresh()
{
    for (GlacierPMBackend* backend : m_backends) {
        backend->update();
    }
}
