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

#include "aptbackendworker.h"

#include <glacierrepo.h>

#include <apt-pkg/acquire.h>
#include <apt-pkg/algorithms.h>
#include <apt-pkg/cachefile.h>
#include <apt-pkg/configuration.h>
#include <apt-pkg/depcache.h>
#include <apt-pkg/error.h>
#include <apt-pkg/init.h>
#include <apt-pkg/metaindex.h>
#include <apt-pkg/packagemanager.h>
#include <apt-pkg/pkgcache.h>
#include <apt-pkg/pkgrecords.h>
#include <apt-pkg/pkgsystem.h>
#include <apt-pkg/policy.h>
#include <apt-pkg/sourcelist.h>
#include <apt-pkg/update.h>

#include <QDebug>
#include <QHash>
#include <QProcess>
#include <QUrl>

#include <functional>

#ifdef HAVE_APPSTREAMQT
#include <AppStreamQt/icon.h>
#include <AppStreamQt/pool.h>
#endif

static bool ensureAptInitialized()
{
    static bool initialized = false;
    static bool ok = false;
    if (initialized) {
        return ok;
    }
    initialized = true;
    ok = pkgInitConfig(*_config) && pkgInitSystem(*_config, _system);
    if (!ok) {
        qWarning() << "Failed to initialize libapt-pkg";
    }
    return ok;
}

static QString candidateVersion(pkgCacheFile& cache, pkgCache::PkgIterator pkg)
{
    pkgDepCache* depCache = cache.GetDepCache();
    if (!depCache) {
        return QString();
    }
    pkgDepCache::StateCache& state = (*depCache)[pkg];
    if (state.CandidateVer == nullptr) {
        return QString();
    }
    return QString::fromUtf8(state.CandidateVerIter(*depCache).VerStr());
}

static QString installedVersion(pkgCache::PkgIterator pkg)
{
    if (pkg->CurrentVer == 0) {
        return QString();
    }
    return QString::fromUtf8(pkg.CurrentVer().VerStr());
}

static QString shortDescription(pkgCacheFile& cache, pkgRecords& records, pkgCache::PkgIterator pkg)
{
    pkgDepCache* depCache = cache.GetDepCache();
    if (!depCache) {
        return QString();
    }
    pkgDepCache::StateCache& state = (*depCache)[pkg];
    pkgCache::VerIterator ver;
    if (state.CandidateVer != nullptr) {
        ver = state.CandidateVerIter(*depCache);
    } else if (pkg->CurrentVer != 0) {
        ver = pkg.CurrentVer();
    }
    if (ver.end()) {
        return QString();
    }
    pkgCache::DescIterator desc = ver.TranslatedDescription();
    if (desc.end()) {
        return QString();
    }
    pkgRecords::Parser& parser = records.Lookup(desc.FileList());
    return QString::fromUtf8(parser.ShortDesc().c_str()).trimmed();
}

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

static GlacierPackage makePackage(pkgCacheFile& cache, pkgRecords& records, pkgCache::PkgIterator pkg)
{
    GlacierPackage package;
    const QString name = QString::fromUtf8(pkg.Name());
    const QString instVer = installedVersion(pkg);
    const QString candVer = candidateVersion(cache, pkg);

    package.name = name;
    package.description = shortDescription(cache, records, pkg);
    package.icon = packageIcon(name);
    package.screenshots = QVariantList();
    package.repo = QStringLiteral("apt");
    package.installed = !instVer.isEmpty();
    package.version = candVer.isEmpty() ? instVer : candVer;
    package.installedVersion = instVer;

    pkgDepCache* depCache = cache.GetDepCache();
    bool upgradable = false;
    if (depCache) {
        upgradable = (*depCache)[pkg].Upgradable();
    }
    package.haveUpdates = upgradable;
    return package;
}

static bool withCache(const std::function<void(pkgCacheFile&)>& fn)
{
    if (!ensureAptInitialized()) {
        return false;
    }

    pkgCacheFile cache;
    if (cache.Open(nullptr, false) == false) {
        qWarning() << "Failed to open apt cache";
        return false;
    }
    fn(cache);
    return true;
}

AptBackendWorker::AptBackendWorker(QObject* parent)
    : QObject(parent)
{
}

void AptBackendWorker::doSearchPackages(const QString& name)
{
    QVariantList results;
    const QString term = name.trimmed();
    if (term.isEmpty()) {
        emit searchPackagesReady(results);
        return;
    }

    withCache([&](pkgCacheFile& cache) {
        pkgCache* pkgCachePtr = cache.GetPkgCache();
        pkgDepCache* depCache = cache.GetDepCache();
        if (!pkgCachePtr || !depCache) {
            return;
        }

        pkgRecords records(*depCache);
        const std::string needle = term.toStdString();
        for (pkgCache::PkgIterator pkg = pkgCachePtr->PkgBegin(); !pkg.end(); ++pkg) {
            const char* pkgName = pkg.Name();
            if (pkgName == nullptr) {
                continue;
            }
            if (std::string(pkgName).find(needle) == std::string::npos) {
                continue;
            }

            results.append(QVariant::fromValue(makePackage(cache, records, pkg)));
            if (results.size() >= 100) {
                break;
            }
        }
    });

    emit searchPackagesReady(results);
}

void AptBackendWorker::doGetUpdates()
{
    QVariantList updates;

    withCache([&](pkgCacheFile& cache) {
        pkgCache* pkgCachePtr = cache.GetPkgCache();
        pkgDepCache* depCache = cache.GetDepCache();
        if (!pkgCachePtr || !depCache) {
            return;
        }

        pkgRecords records(*depCache);
        for (pkgCache::PkgIterator pkg = pkgCachePtr->PkgBegin(); !pkg.end(); ++pkg) {
            if (pkg->CurrentVer == 0) {
                continue;
            }
            pkgDepCache::StateCache& state = (*depCache)[pkg];
            if (state.CandidateVer == nullptr) {
                continue;
            }
            if (!state.Upgradable()) {
                continue;
            }
            const QString current = QString::fromUtf8(pkg.CurrentVer().VerStr());
            const QString candidate = QString::fromUtf8(state.CandidateVerIter(*depCache).VerStr());
            if (current == candidate) {
                continue;
            }
            updates.append(QVariant::fromValue(makePackage(cache, records, pkg)));
        }
    });

    emit getUpdatesReady(updates);
}

void AptBackendWorker::doGetOrphansPackages()
{
    QVariantList orphans;

    withCache([&](pkgCacheFile& cache) {
        pkgCache* pkgCachePtr = cache.GetPkgCache();
        pkgDepCache* depCache = cache.GetDepCache();
        if (!pkgCachePtr || !depCache) {
            return;
        }

        pkgRecords records(*depCache);
        for (pkgCache::PkgIterator pkg = pkgCachePtr->PkgBegin(); !pkg.end(); ++pkg) {
            const pkgDepCache::StateCache& state = (*depCache)[pkg];
            if (pkg->CurrentVer == 0) {
                continue;
            }
            if (!state.Garbage) {
                continue;
            }
            orphans.append(QVariant::fromValue(makePackage(cache, records, pkg)));
        }
    });

    emit getOrphansPackagesReady(orphans);
}

void AptBackendWorker::doGetPackage(const QString& name)
{
    GlacierPackage result;
    withCache([&](pkgCacheFile& cache) {
        pkgCache* pkgCachePtr = cache.GetPkgCache();
        pkgDepCache* depCache = cache.GetDepCache();
        if (!pkgCachePtr || !depCache) {
            return;
        }
        pkgCache::PkgIterator pkg = pkgCachePtr->FindPkg(name.toStdString());
        if (pkg.end()) {
            return;
        }
        pkgRecords records(*depCache);
        result = makePackage(cache, records, pkg);
    });
    emit getPackageReady(result);
}

bool AptBackendWorker::runHelper(const QStringList& args, const QString& actionLabel)
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

void AptBackendWorker::doInstall(const QString& packageName)
{
    if (runHelper({ QStringLiteral("--install"), packageName }, tr("Installing %1").arg(packageName))) {
        emit dbRefreshed();
    } else {
        qWarning() << "Install failed for" << packageName;
    }
}

void AptBackendWorker::doInstallFromLocalFile(const QString& patch)
{
    Q_UNUSED(patch)
    qWarning() << "installFromLocalFile via libapt-pkg is not implemented yet";
}

void AptBackendWorker::doRemove(const QString& packageName)
{
    if (runHelper({ QStringLiteral("--remove"), packageName }, tr("Removing %1").arg(packageName))) {
        emit dbRefreshed();
    } else {
        qWarning() << "Remove failed for" << packageName;
    }
}

void AptBackendWorker::doUpgrade(const QString& packageName)
{
    if (packageName.isEmpty()) {
        if (runHelper({ QStringLiteral("--upgrade-all") }, tr("Upgrading all packages"))) {
            emit dbRefreshed();
        } else {
            qWarning() << "Upgrade all failed";
        }
        return;
    }

    if (runHelper({ QStringLiteral("--upgrade"), packageName }, tr("Upgrading %1").arg(packageName))) {
        emit dbRefreshed();
    } else {
        qWarning() << "Upgrade failed for" << packageName;
    }
}

void AptBackendWorker::doUpdate()
{
    if (runHelper({ QStringLiteral("--update") }, tr("Updating package lists"))) {
        emit dbRefreshed();
    } else {
        qWarning() << "Update failed";
        emit dbRefreshed();
    }
}

void AptBackendWorker::doListRepositories()
{
    QVariantList repos;
    withCache([&](pkgCacheFile& cache) {
        pkgSourceList* list = cache.GetSourceList();
        if (!list) {
            return;
        }

        for (pkgSourceList::const_iterator it = list->begin(); it != list->end(); ++it) {
            metaIndex* index = *it;
            if (!index) {
                continue;
            }
            GlacierRepo repo;
            repo.backend = QStringLiteral("apt");
            repo.enabled = true;
            repo.url = QString::fromStdString(index->GetURI());
            repo.name = QString::fromStdString(index->Describe());
            repo.id = repo.url;
            repos.append(QVariant::fromValue(repo));
        }
    });
    emit listRepositoriesReady(repos);
}

void AptBackendWorker::doAddRepository(const QString& url, const QString& name)
{
    if (runHelper({ QStringLiteral("--add-repo"), name, url }, tr("Adding repository %1").arg(name.isEmpty() ? url : name))) {
        emit dbRefreshed();
    } else {
        qWarning() << "Add repository failed" << url;
    }
}

void AptBackendWorker::doRemoveRepository(const QString& id)
{
    if (runHelper({ QStringLiteral("--remove-repo"), id }, tr("Removing repository"))) {
        emit dbRefreshed();
    } else {
        qWarning() << "Remove repository failed" << id;
    }
}

void AptBackendWorker::doSetRepositoryEnabled(const QString& id, bool enabled)
{
    const QString action = enabled ? QStringLiteral("--enable-repo") : QStringLiteral("--disable-repo");
    if (runHelper({ action, id }, tr("Updating repository"))) {
        emit dbRefreshed();
    } else {
        qWarning() << "Set repository enabled failed" << id << enabled;
    }
}
