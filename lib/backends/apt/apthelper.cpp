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

#include <apt-pkg/acquire-item.h>
#include <apt-pkg/acquire.h>
#include <apt-pkg/algorithms.h>
#include <apt-pkg/cachefile.h>
#include <apt-pkg/configuration.h>
#include <apt-pkg/depcache.h>
#include <apt-pkg/error.h>
#include <apt-pkg/init.h>
#include <apt-pkg/install-progress.h>
#include <apt-pkg/packagemanager.h>
#include <apt-pkg/pkgrecords.h>
#include <apt-pkg/pkgsystem.h>
#include <apt-pkg/sourcelist.h>
#include <apt-pkg/update.h>
#include <apt-pkg/upgrade.h>

#include <iostream>
#include <string>

static void emitStatus(const std::string& stage, double percent, const std::string& message)
{
    std::cout << "STATUS\t" << stage << "\t" << percent << "\t" << message << std::endl;
    std::cout.flush();
}

class HelperAcquireStatus : public pkgAcquireStatus {
public:
    bool Pulse(pkgAcquire* owner) override
    {
        pkgAcquireStatus::Pulse(owner);
        double percent = 0;
        if (TotalBytes > 0) {
            percent = (static_cast<double>(CurrentBytes) * 100.0) / static_cast<double>(TotalBytes);
        }
        std::string msg;
        for (pkgAcquire::ItemCIterator it = owner->ItemsBegin(); it != owner->ItemsEnd(); ++it) {
            if ((*it)->Status == pkgAcquire::Item::StatFetching) {
                msg = (*it)->ShortDesc();
                break;
            }
        }
        emitStatus("downloading", percent, msg);
        return true;
    }

    void Fetch(pkgAcquire::ItemDesc& it) override
    {
        emitStatus("downloading", -1, it.ShortDesc);
    }

    void Done(pkgAcquire::ItemDesc& /*it*/) override
    {
    }

    void Start() override
    {
        pkgAcquireStatus::Start();
        emitStatus("downloading", 0, "Starting download");
    }

    void Stop() override
    {
        pkgAcquireStatus::Stop();
        emitStatus("downloading", 100, "Download finished");
    }

    bool MediaChange(std::string /*Media*/, std::string /*Drive*/) override
    {
        return false;
    }
};

static bool ensureAptInitialized()
{
    return pkgInitConfig(*_config) && pkgInitSystem(*_config, _system);
}

static int runInstall(const std::string& packageName)
{
    if (!ensureAptInitialized()) {
        std::cerr << "Failed to initialize apt" << std::endl;
        return 1;
    }

    pkgCacheFile cache;
    if (cache.Open(nullptr, false) == false) {
        std::cerr << "Failed to open apt cache" << std::endl;
        return 1;
    }

    pkgDepCache* depCache = cache.GetDepCache();
    pkgCache::PkgIterator pkg = cache.GetPkgCache()->FindPkg(packageName);
    if (pkg.end() || !depCache) {
        std::cerr << "Package not found: " << packageName << std::endl;
        return 1;
    }

    emitStatus("preparing", 0, "Resolving dependencies");
    depCache->MarkInstall(pkg, true);
    pkgProblemResolver fixer(depCache);
    fixer.Resolve(true);

    pkgSourceList* list = cache.GetSourceList();
    HelperAcquireStatus fetchStatus;
    pkgAcquire fetcher(&fetchStatus);
    pkgPackageManager* pm = _system->CreatePM(depCache);
    if (!pm || !list) {
        delete pm;
        return 1;
    }

    if (cache.GetDepCache()->DelCount() == 0 && cache.GetDepCache()->InstCount() == 0) {
        delete pm;
        return 0;
    }

    pkgRecords records(*depCache);
    if (!pm->GetArchives(&fetcher, list, &records)) {
        delete pm;
        return 1;
    }

    fetcher.Run();
    emitStatus("installing", 0, "Installing packages");
    APT::Progress::PackageManager progress;
    pkgPackageManager::OrderResult res = pm->DoInstall(&progress);
    delete pm;
    if (res == pkgPackageManager::Completed || res == pkgPackageManager::Incomplete) {
        emitStatus("finished", 100, "Done");
        return 0;
    }
    return 1;
}

static int runRemove(const std::string& packageName)
{
    if (!ensureAptInitialized()) {
        std::cerr << "Failed to initialize apt" << std::endl;
        return 1;
    }

    pkgCacheFile cache;
    if (cache.Open(nullptr, false) == false) {
        std::cerr << "Failed to open apt cache" << std::endl;
        return 1;
    }

    pkgDepCache* depCache = cache.GetDepCache();
    pkgCache::PkgIterator pkg = cache.GetPkgCache()->FindPkg(packageName);
    if (pkg.end() || !depCache) {
        std::cerr << "Package not found: " << packageName << std::endl;
        return 1;
    }

    emitStatus("preparing", 0, "Resolving dependencies");
    depCache->MarkDelete(pkg, false);
    pkgProblemResolver fixer(depCache);
    fixer.Resolve(true);

    pkgSourceList* list = cache.GetSourceList();
    HelperAcquireStatus fetchStatus;
    pkgAcquire fetcher(&fetchStatus);
    pkgPackageManager* pm = _system->CreatePM(depCache);
    if (!pm || !list) {
        delete pm;
        return 1;
    }

    if (cache.GetDepCache()->DelCount() == 0 && cache.GetDepCache()->InstCount() == 0) {
        delete pm;
        return 0;
    }

    pkgRecords records(*depCache);
    if (!pm->GetArchives(&fetcher, list, &records)) {
        delete pm;
        return 1;
    }

    fetcher.Run();
    emitStatus("removing", 0, "Removing packages");
    APT::Progress::PackageManager progress;
    pkgPackageManager::OrderResult res = pm->DoInstall(&progress);
    delete pm;
    if (res == pkgPackageManager::Completed || res == pkgPackageManager::Incomplete) {
        emitStatus("finished", 100, "Done");
        return 0;
    }
    return 1;
}

static int runUpdate()
{
    if (!ensureAptInitialized()) {
        std::cerr << "Failed to initialize apt" << std::endl;
        return 1;
    }

    pkgCacheFile cache;
    if (cache.Open(nullptr, false) == false) {
        std::cerr << "Failed to open apt cache" << std::endl;
        return 1;
    }

    pkgSourceList* list = cache.GetSourceList();
    if (!list) {
        std::cerr << "No source list" << std::endl;
        return 1;
    }

    HelperAcquireStatus fetchStatus;
    pkgAcquire fetcher(&fetchStatus);
    emitStatus("downloading", 0, "Updating package lists");
    if (list->GetIndexes(&fetcher) == false) {
        std::cerr << "Failed to get indexes" << std::endl;
        return 1;
    }

    fetcher.Run();
    emitStatus("finished", 100, "Done");
    return 0;
}

static int runUpgradeAll()
{
    if (!ensureAptInitialized()) {
        std::cerr << "Failed to initialize apt" << std::endl;
        return 1;
    }

    pkgCacheFile cache;
    if (cache.Open(nullptr, false) == false) {
        std::cerr << "Failed to open apt cache" << std::endl;
        return 1;
    }

    pkgDepCache* depCache = cache.GetDepCache();
    if (!depCache) {
        std::cerr << "Failed to get depcache" << std::endl;
        return 1;
    }

    emitStatus("preparing", 0, "Resolving dependencies");
    APT::Upgrade::Upgrade(*depCache, APT::Upgrade::ALLOW_EVERYTHING);

    pkgProblemResolver fixer(depCache);
    fixer.Resolve(true);

    pkgSourceList* list = cache.GetSourceList();
    HelperAcquireStatus fetchStatus;
    pkgAcquire fetcher(&fetchStatus);
    pkgPackageManager* pm = _system->CreatePM(depCache);
    if (!pm || !list) {
        delete pm;
        return 1;
    }

    if (cache.GetDepCache()->DelCount() == 0 && cache.GetDepCache()->InstCount() == 0) {
        delete pm;
        emitStatus("finished", 100, "Done");
        return 0;
    }

    pkgRecords records(*depCache);
    if (!pm->GetArchives(&fetcher, list, &records)) {
        delete pm;
        return 1;
    }

    fetcher.Run();
    emitStatus("installing", 0, "Upgrading packages");
    APT::Progress::PackageManager progress;
    pkgPackageManager::OrderResult res = pm->DoInstall(&progress);
    delete pm;
    if (res == pkgPackageManager::Completed || res == pkgPackageManager::Incomplete) {
        emitStatus("finished", 100, "Done");
        return 0;
    }
    return 1;
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::cerr << "Usage: glacier-packagemanager-helper --install|--remove|--upgrade|--update <package>" << std::endl;
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
    if (action == "--install" || action == "--upgrade") {
        return runInstall(packageName);
    }
    if (action == "--remove") {
        return runRemove(packageName);
    }

    std::cerr << "Unknown action: " << action << std::endl;
    return 2;
}
