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

#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>

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

GlacierPackage FlatpakBackend::findPackageByFile(const QString& filePath)
{
    GlacierPackage result;
    if (filePath.isEmpty()) {
        return result;
    }

    QString resolved = filePath;
    QFileInfo info(resolved);
    if (!info.isAbsolute() || !info.exists()) {
        if (resolved.endsWith(QStringLiteral(".desktop"))) {
            const QStringList appDirs = {
                QDir::homePath() + QStringLiteral("/.local/share/flatpak/exports/share/applications"),
                QStringLiteral("/var/lib/flatpak/exports/share/applications")
            };
            for (const QString& dir : appDirs) {
                const QString candidate = dir + QLatin1Char('/') + QFileInfo(resolved).fileName();
                if (QFileInfo::exists(candidate)) {
                    resolved = candidate;
                    info = QFileInfo(candidate);
                    break;
                }
            }
        } else {
            const QString exe = QStandardPaths::findExecutable(resolved);
            if (!exe.isEmpty()) {
                resolved = exe;
                info = QFileInfo(resolved);
            }
        }
    }

    const QString abs = info.absoluteFilePath();
    const QStringList roots = {
        QStringLiteral("/var/lib/flatpak/app/"),
        QDir::homePath() + QStringLiteral("/.local/share/flatpak/app/")
    };
    for (const QString& root : roots) {
        if (abs.startsWith(root)) {
            const QString rest = abs.mid(root.size());
            const QString appId = rest.section(QLatin1Char('/'), 0, 0);
            if (!appId.isEmpty()) {
                result.name = appId;
                result.repo = QStringLiteral("flatpak");
                result.installed = true;
                return result;
            }
        }
    }

    const QString fileName = info.fileName();
    if (fileName.endsWith(QStringLiteral(".desktop"))) {
        const QString base = fileName.left(fileName.size() - 8);
        if (base.contains(QLatin1Char('.'))) {
            const QStringList exportDirs = {
                QStringLiteral("/var/lib/flatpak/exports/share/applications/"),
                QDir::homePath() + QStringLiteral("/.local/share/flatpak/exports/share/applications/")
            };
            for (const QString& dir : exportDirs) {
                if (abs.startsWith(dir)) {
                    result.name = base;
                    result.repo = QStringLiteral("flatpak");
                    result.installed = true;
                    return result;
                }
            }
        }
    }

    return result;
}
