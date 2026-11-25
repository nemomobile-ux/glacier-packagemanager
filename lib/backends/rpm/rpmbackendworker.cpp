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

#include "rpmbackendworker.h"

#include <glacierrepo.h>

#include <zypp/Package.h>
#include <zypp/Pathname.h>
#include <zypp/RepoInfo.h>
#include <zypp/RepoManager.h>
#include <zypp/ResPool.h>
#include <zypp/ResPoolProxy.h>
#include <zypp/ZYpp.h>
#include <zypp/ZYppFactory.h>
#include <zypp/base/Exception.h>
#include <zypp/ui/Selectable.h>

#include <QDebug>
#include <QHash>
#include <QProcess>
#include <QUrl>

#ifdef HAVE_APPSTREAMQT
#include <AppStreamQt/icon.h>
#include <AppStreamQt/pool.h>
#endif

static QString packageIcon(const QString& packageName)
{
#ifdef HAVE_APPSTREAMQT
    static QHash<QString, QString> iconCache;
    static bool cacheLoaded = false;
    if (!cacheLoaded) {
        cacheLoaded = true;
        AppStream::Pool pool;
        if (!pool.load()) {
            qWarning() << "Failed to load AppStream pool:" << pool.lastError();
            return QString();
        }

        const auto components = pool.components();
        for (const auto& component : components) {
            QString iconPath;
            const auto icons = component.icons();
            for (const auto& icon : icons) {
                if (icon.kind() == AppStream::Icon::KindLocal || icon.kind() == AppStream::Icon::KindCached) {
                    const QString local = icon.url().toLocalFile();
                    if (!local.isEmpty()) {
                        iconPath = QUrl::fromLocalFile(local).toString();
                        break;
                    }
                    const QUrl iconUrl = icon.url();
                    if (iconUrl.isValid() && !iconUrl.isEmpty()) {
                        iconPath = iconUrl.toString();
                        break;
                    }
                }
                if (icon.kind() == AppStream::Icon::KindStock) {
                    const QString stockName = icon.name();
                    if (!stockName.isEmpty()) {
                        iconPath = QStringLiteral("image://theme/") + stockName;
                        break;
                    }
                }
            }
            if (iconPath.isEmpty()) {
                continue;
            }
            const QStringList pkgs = component.packageNames();
            for (const QString& pkg : pkgs) {
                if (!iconCache.contains(pkg)) {
                    iconCache.insert(pkg, iconPath);
                }
            }
        }
    }

    return iconCache.value(packageName);
#else
    Q_UNUSED(packageName)
    return QString();
#endif
}

static GlacierPackage makePackage(const zypp::ui::Selectable::Ptr& selectable)
{
    GlacierPackage package;
    if (!selectable) {
        return package;
    }

    const QString name = QString::fromStdString(selectable->name());
    package.name = name;
    package.icon = packageIcon(name);
    package.repo = QStringLiteral("rpm");
    package.screenshots = QVariantList();

    zypp::PoolItem installed = selectable->installedObj();
    zypp::PoolItem candidate = selectable->candidateObj();
    zypp::PoolItem update = selectable->updateCandidateObj();

    if (installed) {
        package.installed = true;
        package.installedVersion = QString::fromStdString(installed.edition().asString());
        package.description = QString::fromStdString(installed.summary());
    }

    if (update) {
        package.haveUpdates = true;
        package.version = QString::fromStdString(update.edition().asString());
        if (package.description.isEmpty()) {
            package.description = QString::fromStdString(update.summary());
        }
    } else if (candidate) {
        package.version = QString::fromStdString(candidate.edition().asString());
        if (package.description.isEmpty()) {
            package.description = QString::fromStdString(candidate.summary());
        }
    }

    if (package.version.isEmpty()) {
        package.version = package.installedVersion;
    }

    return package;
}

RpmBackendWorker::RpmBackendWorker(QObject* parent)
    : QObject(parent)
{
}

bool RpmBackendWorker::ensureZypp(bool loadRepos)
{
    try {
        zypp::ZYpp::Ptr z = zypp::getZYpp();
        if (!m_zyppInitialized) {
            z->initializeTarget(zypp::Pathname("/"));
            z->target()->load();
            m_zyppInitialized = true;
        }

        if (loadRepos && !m_reposLoaded) {
            zypp::RepoManager repoManager;
            const auto repos = repoManager.knownRepositories();
            for (const zypp::RepoInfo& repo : repos) {
                if (!repo.enabled()) {
                    continue;
                }
                try {
                    repoManager.loadFromCache(repo);
                } catch (const zypp::Exception& e) {
                    qWarning() << "Failed to load repo cache:" << QString::fromStdString(repo.alias()) << e.asUserHistory().c_str();
                }
            }
            m_reposLoaded = true;
        }
        return true;
    } catch (const zypp::Exception& e) {
        qWarning() << "ZYpp init failed:" << e.asUserHistory().c_str();
        return false;
    } catch (...) {
        qWarning() << "ZYpp init failed: unknown error";
        return false;
    }
}

void RpmBackendWorker::doSearchPackages(const QString& name)
{
    QVariantList results;
    if (name.trimmed().isEmpty()) {
        emit searchPackagesReady(results);
        return;
    }

    if (!ensureZypp(true)) {
        emit searchPackagesReady(results);
        return;
    }

    const QString needle = name.trimmed();
    try {
        zypp::ResPoolProxy proxy = zypp::getZYpp()->poolProxy();
        for (const zypp::ui::Selectable::Ptr& selectable : proxy.byKind<zypp::Package>()) {
            if (!selectable) {
                continue;
            }
            const QString pkgName = QString::fromStdString(selectable->name());
            QString summary;
            if (selectable->candidateObj()) {
                summary = QString::fromStdString(selectable->candidateObj().summary());
            } else if (selectable->installedObj()) {
                summary = QString::fromStdString(selectable->installedObj().summary());
            }
            if (pkgName.contains(needle, Qt::CaseInsensitive) || summary.contains(needle, Qt::CaseInsensitive)) {
                results.append(QVariant::fromValue(makePackage(selectable)));
            }
        }
    } catch (const zypp::Exception& e) {
        qWarning() << "searchPackages failed:" << e.asUserHistory().c_str();
    }

    emit searchPackagesReady(results);
}

void RpmBackendWorker::doGetUpdates()
{
    QVariantList updates;
    if (!ensureZypp(true)) {
        emit getUpdatesReady(updates);
        return;
    }

    try {
        zypp::ResPoolProxy proxy = zypp::getZYpp()->poolProxy();
        for (const zypp::ui::Selectable::Ptr& selectable : proxy.byKind<zypp::Package>()) {
            if (selectable && selectable->hasInstalledObj() && selectable->updateCandidateObj()) {
                updates.append(QVariant::fromValue(makePackage(selectable)));
            }
        }
    } catch (const zypp::Exception& e) {
        qWarning() << "getUpdates failed:" << e.asUserHistory().c_str();
    }

    emit getUpdatesReady(updates);
}

void RpmBackendWorker::doGetOrphansPackages()
{
    QVariantList orphans;
    if (!ensureZypp(true)) {
        emit getOrphansPackagesReady(orphans);
        return;
    }

    try {
        zypp::ResPoolProxy proxy = zypp::getZYpp()->poolProxy();
        for (const zypp::ui::Selectable::Ptr& selectable : proxy.byKind<zypp::Package>()) {
            if (!selectable || !selectable->hasInstalledObj()) {
                continue;
            }
            zypp::PoolItem installed = selectable->installedObj();
            if (installed.status().isUnneeded()) {
                orphans.append(QVariant::fromValue(makePackage(selectable)));
            }
        }
    } catch (const zypp::Exception& e) {
        qWarning() << "getOrphansPackages failed:" << e.asUserHistory().c_str();
    }

    emit getOrphansPackagesReady(orphans);
}

void RpmBackendWorker::doGetPackage(const QString& name)
{
    GlacierPackage result;
    if (!ensureZypp(true)) {
        emit getPackageReady(result);
        return;
    }

    try {
        zypp::ResPoolProxy proxy = zypp::getZYpp()->poolProxy();
        zypp::ui::Selectable::Ptr selectable = proxy.lookup(zypp::ResKind::package, name.toStdString());
        if (selectable) {
            result = makePackage(selectable);
        }
    } catch (const zypp::Exception& e) {
        qWarning() << "getPackage failed:" << e.asUserHistory().c_str();
    }

    emit getPackageReady(result);
}

bool RpmBackendWorker::runHelper(const QStringList& args, const QString& actionLabel)
{
    emit emitAction(actionLabel);

    QProcess process;
    process.setProgram(QStringLiteral("pkexec"));
    process.setArguments(QStringList { QStringLiteral("/usr/libexec/glacier-packagemanager-helper") } + args);
    process.setProcessChannelMode(QProcess::MergedChannels);
    process.start();

    QByteArray buffer;
    QByteArray allOutput;
    while (process.state() != QProcess::NotRunning || process.bytesAvailable() > 0) {
        if (!process.waitForReadyRead(200)) {
            if (process.state() == QProcess::NotRunning) {
                break;
            }
            continue;
        }
        const QByteArray chunk = process.readAll();
        buffer.append(chunk);
        allOutput.append(chunk);
        while (true) {
            int idx = buffer.indexOf('\n');
            if (idx < 0) {
                break;
            }
            const QString line = QString::fromUtf8(buffer.left(idx)).trimmed();
            buffer.remove(0, idx + 1);
            if (!line.startsWith(QStringLiteral("STATUS\t"))) {
                continue;
            }
            const QStringList parts = line.split(QLatin1Char('\t'));
            if (parts.size() < 4) {
                continue;
            }
            const QString stage = parts.at(1);
            const double percent = parts.at(2).toDouble();
            const QString message = parts.mid(3).join(QLatin1Char('\t'));
            emit emitActionProgress(stage, message, percent < 0 ? 0 : percent);
        }
    }
    process.waitForFinished();
    allOutput.append(process.readAll());

    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        qWarning() << "Helper failed" << args << "exit" << process.exitCode() << process.exitStatus() << QString::fromUtf8(allOutput);
    }

    return process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0;
}

void RpmBackendWorker::doInstall(const QString& packageName)
{
    if (runHelper({ QStringLiteral("--install"), packageName }, tr("Installing %1").arg(packageName))) {
        m_reposLoaded = false;
        emit dbRefreshed();
    } else {
        qWarning() << "Install failed for" << packageName;
    }
}

void RpmBackendWorker::doInstallFromLocalFile(const QString& patch)
{
    Q_UNUSED(patch)
    qWarning() << "installFromLocalFile via libzypp is not implemented yet";
}

void RpmBackendWorker::doRemove(const QString& packageName)
{
    if (runHelper({ QStringLiteral("--remove"), packageName }, tr("Removing %1").arg(packageName))) {
        m_reposLoaded = false;
        emit dbRefreshed();
    } else {
        qWarning() << "Remove failed for" << packageName;
    }
}

void RpmBackendWorker::doUpgrade(const QString& packageName)
{
    if (packageName.isEmpty()) {
        if (runHelper({ QStringLiteral("--upgrade-all") }, tr("Upgrading all packages"))) {
            m_reposLoaded = false;
            emit dbRefreshed();
        } else {
            qWarning() << "Upgrade all failed";
        }
        return;
    }

    if (runHelper({ QStringLiteral("--upgrade"), packageName }, tr("Upgrading %1").arg(packageName))) {
        m_reposLoaded = false;
        emit dbRefreshed();
    } else {
        qWarning() << "Upgrade failed for" << packageName;
    }
}

void RpmBackendWorker::doUpdate()
{
    if (runHelper({ QStringLiteral("--update") }, tr("Updating package lists"))) {
        m_reposLoaded = false;
        emit dbRefreshed();
    } else {
        qWarning() << "Update failed";
        emit dbRefreshed();
    }
}

void RpmBackendWorker::doListRepositories()
{
    QVariantList repos;
    try {
        zypp::RepoManager repoManager;
        const auto known = repoManager.knownRepositories();
        for (const zypp::RepoInfo& info : known) {
            GlacierRepo repo;
            repo.id = QString::fromStdString(info.alias());
            repo.name = QString::fromStdString(info.name().empty() ? info.alias() : info.name());
            repo.url = QString::fromStdString(info.url().asString());
            repo.backend = QStringLiteral("rpm");
            repo.enabled = info.enabled();
            repos.append(QVariant::fromValue(repo));
        }
    } catch (const zypp::Exception& e) {
        qWarning() << "listRepositories failed:" << e.asUserHistory().c_str();
    }
    emit listRepositoriesReady(repos);
}
