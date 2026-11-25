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

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
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

static std::string slugify(const std::string& input)
{
    std::string out;
    for (char c : input) {
        if (std::isalnum(static_cast<unsigned char>(c))) {
            out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
        } else if (!out.empty() && out.back() != '-') {
            out.push_back('-');
        }
    }
    while (!out.empty() && out.back() == '-') {
        out.pop_back();
    }
    if (out.empty()) {
        out = "repo";
    }
    return out;
}

static int runAddRepo(const std::string& name, const std::string& url)
{
    emitStatus("preparing", 0, "Adding repository");
    const std::string slug = slugify(name.empty() ? url : name);
    const std::filesystem::path dir = "/etc/apt/sources.list.d";
    std::filesystem::create_directories(dir);
    const std::filesystem::path file = dir / ("glacier-" + slug + ".list");

    std::string line = url;
    if (line.find(' ') == std::string::npos) {
        line = "deb " + url + " stable main";
    }

    std::ofstream out(file, std::ios::app);
    if (!out.is_open()) {
        std::cerr << "Failed to open repo file: " << file << std::endl;
        return 1;
    }
    out << line << std::endl;
    out.close();
    emitStatus("finished", 100, "Done");
    return 0;
}

static int runRemoveRepo(const std::string& id)
{
    emitStatus("preparing", 0, "Removing repository");
    const std::filesystem::path dir = "/etc/apt/sources.list.d";
    bool removed = false;
    if (std::filesystem::exists(dir)) {
        for (const auto& entry : std::filesystem::directory_iterator(dir)) {
            if (!entry.is_regular_file()) {
                continue;
            }
            const std::string path = entry.path().string();
            std::ifstream in(path);
            if (!in.is_open()) {
                continue;
            }
            std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
            in.close();
            if (content.find(id) != std::string::npos) {
                std::filesystem::remove(entry.path());
                removed = true;
            }
        }
    }
    if (!removed) {
        std::cerr << "Repository not found in managed files: " << id << std::endl;
    }
    emitStatus("finished", 100, "Done");
    return removed ? 0 : 1;
}

static int runSetRepoEnabled(const std::string& id, bool enabled)
{
    emitStatus("preparing", 0, "Updating repository");
    const std::filesystem::path dir = "/etc/apt/sources.list.d";
    bool changed = false;
    if (std::filesystem::exists(dir)) {
        for (const auto& entry : std::filesystem::directory_iterator(dir)) {
            if (!entry.is_regular_file()) {
                continue;
            }
            std::ifstream in(entry.path());
            if (!in.is_open()) {
                continue;
            }
            std::string line;
            std::string outContent;
            bool fileChanged = false;
            while (std::getline(in, line)) {
                if (line.find(id) != std::string::npos) {
                    std::string trimmed = line;
                    while (!trimmed.empty() && (trimmed.front() == ' ' || trimmed.front() == '\t')) {
                        trimmed.erase(trimmed.begin());
                    }
                    if (enabled) {
                        if (trimmed.rfind("#", 0) == 0) {
                            while (!trimmed.empty() && (trimmed.front() == '#' || trimmed.front() == ' ' || trimmed.front() == '\t')) {
                                trimmed.erase(trimmed.begin());
                            }
                            line = trimmed;
                            fileChanged = true;
                        }
                    } else {
                        if (trimmed.rfind("#", 0) != 0 && !trimmed.empty()) {
                            line = "# " + trimmed;
                            fileChanged = true;
                        }
                    }
                }
                outContent += line + "\n";
            }
            in.close();
            if (fileChanged) {
                std::ofstream out(entry.path(), std::ios::trunc);
                out << outContent;
                out.close();
                changed = true;
            }
        }
    }
    if (!changed) {
        std::cerr << "Repository not found or unchanged: " << id << std::endl;
    }
    emitStatus("finished", 100, "Done");
    return changed ? 0 : 1;
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::cerr << "Usage: glacier-packagemanager-helper --install|--remove|--upgrade|--upgrade-all|--update|--add-repo|--remove-repo|--enable-repo|--disable-repo <args>" << std::endl;
        return 2;
    }

    const std::string action = argv[1];
    if (action == "--update") {
        return runUpdate();
    }

    if (action == "--upgrade-all") {
        return runUpgradeAll();
    }

    if (action == "--add-repo") {
        if (argc < 4) {
            std::cerr << "Usage: --add-repo <name> <url>" << std::endl;
            return 2;
        }
        return runAddRepo(argv[2], argv[3]);
    }

    if (action == "--remove-repo") {
        if (argc < 3) {
            std::cerr << "Repository id required" << std::endl;
            return 2;
        }
        return runRemoveRepo(argv[2]);
    }

    if (action == "--enable-repo" || action == "--disable-repo") {
        if (argc < 3) {
            std::cerr << "Repository id required" << std::endl;
            return 2;
        }
        return runSetRepoEnabled(argv[2], action == "--enable-repo");
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
