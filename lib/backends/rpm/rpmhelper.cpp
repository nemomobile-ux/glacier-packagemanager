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

#include <zypp/Package.h>
#include <zypp/Pathname.h>
#include <zypp/RepoInfo.h>
#include <zypp/RepoManager.h>
#include <zypp/ResPool.h>
#include <zypp/ResPoolProxy.h>
#include <zypp/ZYpp.h>
#include <zypp/ZYppCommit.h>
#include <zypp/ZYppFactory.h>
#include <zypp/base/Exception.h>
#include <zypp/ui/Selectable.h>

#include <iostream>
#include <string>

static void emitStatus(const std::string& stage, int percent, const std::string& message)
{
    std::cout << "STATUS\t" << stage << "\t" << percent << "\t" << message << std::endl;
    std::cout.flush();
}

static bool initZypp(zypp::ZYpp::Ptr& z, bool loadRepos)
{
    try {
        z = zypp::getZYpp();
        z->initializeTarget(zypp::Pathname("/"));
        z->target()->load();

        if (loadRepos) {
            zypp::RepoManager repoManager;
            const auto repos = repoManager.knownRepositories();
            for (const zypp::RepoInfo& repo : repos) {
                if (!repo.enabled()) {
                    continue;
                }
                try {
                    repoManager.loadFromCache(repo);
                } catch (const zypp::Exception&) {
                }
            }
        }
        return true;
    } catch (const zypp::Exception& e) {
        std::cerr << e.asUserHistory() << std::endl;
        return false;
    }
}

static int commitChanges(zypp::ZYpp::Ptr& z)
{
    emitStatus("preparing", 0, "Resolving dependencies");
    if (!z->resolver()->resolvePool()) {
        std::cerr << "Dependency resolution failed" << std::endl;
        return 1;
    }

    zypp::ZYppCommitPolicy policy;
    policy.downloadMode(zypp::DownloadInAdvance);
    policy.syncPoolAfterCommit(true);

    emitStatus("downloading", 0, "Downloading packages");
    zypp::ZYppCommitResult result = z->commit(policy);
    if (!result.allDone()) {
        std::cerr << "Commit failed" << std::endl;
        emitStatus("installing", 100, "Failed");
        return 1;
    }

    emitStatus("finished", 100, "Done");
    return 0;
}

static int runInstall(const std::string& packageName)
{
    zypp::ZYpp::Ptr z;
    if (!initZypp(z, true)) {
        return 1;
    }

    emitStatus("preparing", 0, "Resolving dependencies");
    zypp::ui::Selectable::Ptr selectable = z->poolProxy().lookup(zypp::ResKind::package, packageName);
    if (!selectable) {
        std::cerr << "Package not found: " << packageName << std::endl;
        return 1;
    }

    if (!selectable->setToInstall(zypp::ResStatus::USER)) {
        std::cerr << "Failed to select package for install: " << packageName << std::endl;
        return 1;
    }

    emitStatus("installing", 0, "Installing packages");
    return commitChanges(z);
}

static int runRemove(const std::string& packageName)
{
    zypp::ZYpp::Ptr z;
    if (!initZypp(z, false)) {
        return 1;
    }

    emitStatus("preparing", 0, "Resolving dependencies");
    zypp::ui::Selectable::Ptr selectable = z->poolProxy().lookup(zypp::ResKind::package, packageName);
    if (!selectable) {
        std::cerr << "Package not found: " << packageName << std::endl;
        return 1;
    }

    if (!selectable->setToDelete(zypp::ResStatus::USER)) {
        std::cerr << "Failed to select package for removal: " << packageName << std::endl;
        return 1;
    }

    emitStatus("removing", 0, "Removing packages");
    return commitChanges(z);
}

static int runUpgrade(const std::string& packageName)
{
    zypp::ZYpp::Ptr z;
    if (!initZypp(z, true)) {
        return 1;
    }

    emitStatus("preparing", 0, "Resolving dependencies");
    zypp::ui::Selectable::Ptr selectable = z->poolProxy().lookup(zypp::ResKind::package, packageName);
    if (!selectable) {
        std::cerr << "Package not found: " << packageName << std::endl;
        return 1;
    }

    if (!selectable->setToInstall(zypp::ResStatus::USER)) {
        std::cerr << "Failed to select package for upgrade: " << packageName << std::endl;
        return 1;
    }

    emitStatus("installing", 0, "Upgrading packages");
    return commitChanges(z);
}

static int runUpgradeAll()
{
    zypp::ZYpp::Ptr z;
    if (!initZypp(z, true)) {
        return 1;
    }

    emitStatus("preparing", 0, "Resolving dependencies");
    bool any = false;
    for (const zypp::ui::Selectable::Ptr& selectable : z->poolProxy().byKind<zypp::Package>()) {
        if (selectable && selectable->hasInstalledObj() && selectable->updateCandidateObj()) {
            selectable->setToInstall(zypp::ResStatus::USER);
            any = true;
        }
    }

    if (!any) {
        emitStatus("finished", 100, "Done");
        return 0;
    }

    emitStatus("installing", 0, "Upgrading packages");
    return commitChanges(z);
}

static int runUpdate()
{
    zypp::ZYpp::Ptr z;
    if (!initZypp(z, false)) {
        return 1;
    }

    emitStatus("downloading", 0, "Updating package lists");
    try {
        zypp::RepoManager repoManager;
        const auto repos = repoManager.knownRepositories();
        size_t total = 0;
        for (const zypp::RepoInfo& repo : repos) {
            if (repo.enabled()) {
                total++;
            }
        }
        size_t current = 0;
        for (const zypp::RepoInfo& repo : repos) {
            if (!repo.enabled()) {
                continue;
            }
            try {
                repoManager.refreshMetadata(repo);
                repoManager.buildCache(repo);
            } catch (const zypp::Exception& e) {
                std::cerr << "Failed to refresh " << repo.alias() << ": " << e.asUserHistory() << std::endl;
            }
            current++;
            int percent = total > 0 ? static_cast<int>((current * 100) / total) : 100;
            emitStatus("downloading", percent, repo.alias());
        }
    } catch (const zypp::Exception& e) {
        std::cerr << e.asUserHistory() << std::endl;
        return 1;
    }

    emitStatus("finished", 100, "Done");
    return 0;
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cerr << "Usage: glacier-packagemanager-rpm-helper --install|--remove|--upgrade|--upgrade-all|--update <package>" << std::endl;
        return 2;
    }

    const std::string action = argv[1];
    if (action == "--update") {
        return runUpdate();
    }
    if (action == "--upgrade-all") {
        return runUpgradeAll();
    }

    if (argc < 3) {
        std::cerr << "Package name required" << std::endl;
        return 2;
    }

    const std::string packageName = argv[2];
    if (action == "--install") {
        return runInstall(packageName);
    }
    if (action == "--remove") {
        return runRemove(packageName);
    }
    if (action == "--upgrade") {
        return runUpgrade(packageName);
    }

    std::cerr << "Unknown action: " << action << std::endl;
    return 2;
}
