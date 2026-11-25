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

#include "flatpakbackendworker.h"

#include <glacierrepo.h>

#include <flatpak.h>

#include <QDebug>

static QString toQString(const char* value)
{
    return value ? QString::fromUtf8(value) : QString();
}

static GlacierPackage packageFromInstalledRef(FlatpakInstalledRef* ref)
{
    GlacierPackage pkg;
    pkg.repo = QStringLiteral("flatpak");
    pkg.installed = true;
    pkg.name = toQString(flatpak_ref_get_name(FLATPAK_REF(ref)));
    pkg.installedVersion = toQString(flatpak_ref_get_commit(FLATPAK_REF(ref)));
    pkg.version = pkg.installedVersion;

    const char* appName = flatpak_installed_ref_get_appdata_name(ref);
    const char* summary = flatpak_installed_ref_get_appdata_summary(ref);
    const char* version = flatpak_installed_ref_get_appdata_version(ref);
    if (appName && *appName) {
        pkg.description = toQString(summary);
        pkg.name = toQString(appName);
    } else if (summary && *summary) {
        pkg.description = toQString(summary);
    }
    if (version && *version) {
        pkg.installedVersion = toQString(version);
        pkg.version = pkg.installedVersion;
    }
    return pkg;
}

static GlacierPackage packageFromRemoteRef(FlatpakRemoteRef* ref)
{
    GlacierPackage pkg;
    pkg.repo = QStringLiteral("flatpak");
    pkg.installed = false;
    pkg.name = toQString(flatpak_ref_get_name(FLATPAK_REF(ref)));

    const char* appName = flatpak_remote_ref_get_appdata_name(ref);
    const char* summary = flatpak_remote_ref_get_appdata_summary(ref);
    const char* version = flatpak_remote_ref_get_appdata_version(ref);
    if (appName && *appName) {
        pkg.name = toQString(appName);
    }
    if (summary && *summary) {
        pkg.description = toQString(summary);
    }
    if (version && *version) {
        pkg.version = toQString(version);
    }
    if (pkg.version.isEmpty()) {
        pkg.version = toQString(flatpak_ref_get_commit(FLATPAK_REF(ref)));
    }
    return pkg;
}

FlatpakBackendWorker::FlatpakBackendWorker(QObject* parent)
    : QObject(parent)
{
}

FlatpakBackendWorker::~FlatpakBackendWorker()
{
    if (m_installation) {
        g_object_unref(m_installation);
        m_installation = nullptr;
    }
}

bool FlatpakBackendWorker::ensureInstallation()
{
    if (m_installation) {
        return true;
    }

    GError* error = nullptr;
    m_installation = flatpak_installation_new_system(nullptr, &error);
    if (!m_installation) {
        qWarning() << "Flatpak system installation failed:" << (error ? error->message : "unknown error");
        g_clear_error(&error);
        m_installation = flatpak_installation_new_user(nullptr, &error);
        if (!m_installation) {
            qWarning() << "Flatpak user installation failed:" << (error ? error->message : "unknown error");
            g_clear_error(&error);
            return false;
        }
    }
    return true;
}

void FlatpakBackendWorker::doSearchPackages(const QString& name)
{
    QVariantList results;
    if (name.trimmed().isEmpty() || !ensureInstallation()) {
        emit searchPackagesReady(results);
        return;
    }

    GError* error = nullptr;
    GPtrArray* refs = flatpak_installation_list_remote_refs_sync(m_installation, nullptr, nullptr, &error);
    if (!refs) {
        qWarning() << "Flatpak search failed:" << (error ? error->message : "unknown error");
        g_clear_error(&error);
        emit searchPackagesReady(results);
        return;
    }

    const QString needle = name.trimmed();
    for (guint i = 0; i < refs->len; ++i) {
        auto* ref = FLATPAK_REMOTE_REF(g_ptr_array_index(refs, i));
        if (!ref) {
            continue;
        }
        if (flatpak_ref_get_kind(FLATPAK_REF(ref)) != FLATPAK_REF_KIND_APP) {
            continue;
        }
        const QString refName = toQString(flatpak_ref_get_name(FLATPAK_REF(ref)));
        const QString appName = toQString(flatpak_remote_ref_get_appdata_name(ref));
        const QString summary = toQString(flatpak_remote_ref_get_appdata_summary(ref));
        if (refName.contains(needle, Qt::CaseInsensitive)
            || appName.contains(needle, Qt::CaseInsensitive)
            || summary.contains(needle, Qt::CaseInsensitive)) {
            results.append(QVariant::fromValue(packageFromRemoteRef(ref)));
        }
    }
    g_ptr_array_unref(refs);
    emit searchPackagesReady(results);
}

void FlatpakBackendWorker::doGetUpdates()
{
    QVariantList updates;
    if (!ensureInstallation()) {
        emit getUpdatesReady(updates);
        return;
    }

    GError* error = nullptr;
    GPtrArray* refs = flatpak_installation_list_installed_refs_for_update_sync(m_installation, nullptr, &error);
    if (!refs) {
        qWarning() << "Flatpak updates failed:" << (error ? error->message : "unknown error");
        g_clear_error(&error);
        emit getUpdatesReady(updates);
        return;
    }

    for (guint i = 0; i < refs->len; ++i) {
        auto* ref = FLATPAK_INSTALLED_REF(g_ptr_array_index(refs, i));
        if (!ref) {
            continue;
        }
        if (flatpak_ref_get_kind(FLATPAK_REF(ref)) != FLATPAK_REF_KIND_APP) {
            continue;
        }
        GlacierPackage pkg = packageFromInstalledRef(ref);
        pkg.haveUpdates = true;
        updates.append(QVariant::fromValue(pkg));
    }
    g_ptr_array_unref(refs);
    emit getUpdatesReady(updates);
}

void FlatpakBackendWorker::doGetOrphansPackages()
{
    emit getOrphansPackagesReady(QVariantList());
}

void FlatpakBackendWorker::doGetPackage(const QString& name)
{
    GlacierPackage result;
    if (!ensureInstallation()) {
        emit getPackageReady(result);
        return;
    }

    GError* error = nullptr;
    GPtrArray* installed = flatpak_installation_list_installed_refs_sync(m_installation, nullptr, &error);
    if (installed) {
        for (guint i = 0; i < installed->len; ++i) {
            auto* ref = FLATPAK_INSTALLED_REF(g_ptr_array_index(installed, i));
            if (!ref || flatpak_ref_get_kind(FLATPAK_REF(ref)) != FLATPAK_REF_KIND_APP) {
                continue;
            }
            const QString refName = toQString(flatpak_ref_get_name(FLATPAK_REF(ref)));
            const QString appName = toQString(flatpak_installed_ref_get_appdata_name(ref));
            if (refName == name || appName == name) {
                result = packageFromInstalledRef(ref);
                g_ptr_array_unref(installed);
                emit getPackageReady(result);
                return;
            }
        }
        g_ptr_array_unref(installed);
    } else {
        g_clear_error(&error);
    }

    GPtrArray* remotes = flatpak_installation_list_remote_refs_sync(m_installation, nullptr, nullptr, &error);
    if (remotes) {
        for (guint i = 0; i < remotes->len; ++i) {
            auto* ref = FLATPAK_REMOTE_REF(g_ptr_array_index(remotes, i));
            if (!ref || flatpak_ref_get_kind(FLATPAK_REF(ref)) != FLATPAK_REF_KIND_APP) {
                continue;
            }
            const QString refName = toQString(flatpak_ref_get_name(FLATPAK_REF(ref)));
            const QString appName = toQString(flatpak_remote_ref_get_appdata_name(ref));
            if (refName == name || appName == name) {
                result = packageFromRemoteRef(ref);
                g_ptr_array_unref(remotes);
                emit getPackageReady(result);
                return;
            }
        }
        g_ptr_array_unref(remotes);
    } else {
        g_clear_error(&error);
    }

    emit getPackageReady(result);
}

void FlatpakBackendWorker::doListRepositories()
{
    QVariantList repos;
    if (!ensureInstallation()) {
        emit listRepositoriesReady(repos);
        return;
    }

    GError* error = nullptr;
    GPtrArray* remotes = flatpak_installation_list_remotes(m_installation, nullptr, &error);
    if (!remotes) {
        qWarning() << "Flatpak list remotes failed:" << (error ? error->message : "unknown error");
        g_clear_error(&error);
        emit listRepositoriesReady(repos);
        return;
    }

    for (guint i = 0; i < remotes->len; ++i) {
        auto* remote = FLATPAK_REMOTE(g_ptr_array_index(remotes, i));
        if (!remote) {
            continue;
        }
        GlacierRepo repo;
        repo.id = toQString(flatpak_remote_get_name(remote));
        repo.name = toQString(flatpak_remote_get_title(remote));
        if (repo.name.isEmpty()) {
            repo.name = repo.id;
        }
        repo.url = toQString(flatpak_remote_get_url(remote));
        repo.backend = QStringLiteral("flatpak");
        repo.enabled = !flatpak_remote_get_disabled(remote);
        repos.append(QVariant::fromValue(repo));
    }
    g_ptr_array_unref(remotes);
    emit listRepositoriesReady(repos);
}

void FlatpakBackendWorker::doInstall(const QString& packageName)
{
    if (!ensureInstallation()) {
        return;
    }
    emit emitAction(tr("Installing %1").arg(packageName));
    GError* error = nullptr;
    FlatpakInstalledRef* ref = flatpak_installation_install(m_installation,
        QStringLiteral("flathub").toUtf8().constData(),
        FLATPAK_REF_KIND_APP,
        packageName.toUtf8().constData(),
        nullptr, nullptr,
        nullptr, nullptr, nullptr, &error);
    if (ref) {
        g_object_unref(ref);
        emit dbRefreshed();
    } else {
        qWarning() << "Flatpak install failed:" << packageName << (error ? error->message : "unknown error");
        g_clear_error(&error);
    }
}

void FlatpakBackendWorker::doInstallFromLocalFile(const QString& patch)
{
    Q_UNUSED(patch)
    qWarning() << "installFromLocalFile is not supported for flatpak backend yet";
}

void FlatpakBackendWorker::doRemove(const QString& packageName)
{
    if (!ensureInstallation()) {
        return;
    }
    emit emitAction(tr("Removing %1").arg(packageName));
    GError* error = nullptr;
    gboolean ok = flatpak_installation_uninstall(m_installation,
        FLATPAK_REF_KIND_APP,
        packageName.toUtf8().constData(),
        nullptr, nullptr,
        nullptr, nullptr, nullptr, &error);
    if (ok) {
        emit dbRefreshed();
    } else {
        qWarning() << "Flatpak remove failed:" << packageName << (error ? error->message : "unknown error");
        g_clear_error(&error);
    }
}

void FlatpakBackendWorker::doUpgrade(const QString& packageName)
{
    if (!ensureInstallation()) {
        return;
    }

    if (packageName.isEmpty()) {
        emit emitAction(tr("Upgrading all packages"));
        GError* error = nullptr;
        GPtrArray* refs = flatpak_installation_list_installed_refs_for_update_sync(m_installation, nullptr, &error);
        if (!refs) {
            qWarning() << "Flatpak upgrade-all list failed:" << (error ? error->message : "unknown error");
            g_clear_error(&error);
            return;
        }
        bool anyOk = false;
        for (guint i = 0; i < refs->len; ++i) {
            auto* iref = FLATPAK_INSTALLED_REF(g_ptr_array_index(refs, i));
            if (!iref || flatpak_ref_get_kind(FLATPAK_REF(iref)) != FLATPAK_REF_KIND_APP) {
                continue;
            }
            const char* n = flatpak_ref_get_name(FLATPAK_REF(iref));
            GError* uerr = nullptr;
            FlatpakInstalledRef* updated = flatpak_installation_update(m_installation,
                FLATPAK_UPDATE_FLAGS_NONE,
                FLATPAK_REF_KIND_APP,
                n, nullptr, nullptr,
                nullptr, nullptr, nullptr, &uerr);
            if (updated) {
                g_object_unref(updated);
                anyOk = true;
            } else {
                g_clear_error(&uerr);
            }
        }
        g_ptr_array_unref(refs);
        if (anyOk) {
            emit dbRefreshed();
        }
        return;
    }

    emit emitAction(tr("Upgrading %1").arg(packageName));
    GError* error = nullptr;
    FlatpakInstalledRef* updated = flatpak_installation_update(m_installation,
        FLATPAK_UPDATE_FLAGS_NONE,
        FLATPAK_REF_KIND_APP,
        packageName.toUtf8().constData(),
        nullptr, nullptr,
        nullptr, nullptr, nullptr, &error);
    if (updated) {
        g_object_unref(updated);
        emit dbRefreshed();
    } else {
        qWarning() << "Flatpak upgrade failed:" << packageName << (error ? error->message : "unknown error");
        g_clear_error(&error);
    }
}

void FlatpakBackendWorker::doUpdate()
{
    emit dbRefreshed();
}
