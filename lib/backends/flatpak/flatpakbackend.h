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

#ifndef FLATPAKBACKEND_H
#define FLATPAKBACKEND_H

#include <QObject>
#include <QThread>
#include <glacierpmbackend.h>

class FlatpakBackendWorker;

class FlatpakBackend : public GlacierPMBackend {
    Q_OBJECT
    Q_INTERFACES(GlacierPMBackend)
    Q_PLUGIN_METADATA(IID "GlacierPackageManager.Backend")

public:
    FlatpakBackend();

    QString backendId() const override;
    bool enabled() override;
    void install(QString packageName) override;
    void installFromLocalFile(QString patch) override;
    void remove(QString packageName) override;
    void upgrade(QString packageName) override;
    void update() override;

    void searchPackages(const QString& name) override;
    void getUpdates() override;
    void getOrphansPackages() override;
    void getPackage(const QString& name) override;
    void listRepositories() override;

    ~FlatpakBackend();

signals:
    void requestSearchPackages(const QString& name);
    void requestGetUpdates();
    void requestGetOrphansPackages();
    void requestGetPackage(const QString& name);
    void requestListRepositories();
    void requestInstall(const QString& packageName);
    void requestInstallFromLocalFile(const QString& patch);
    void requestRemove(const QString& packageName);
    void requestUpgrade(const QString& packageName);
    void requestUpdate();

private:
    QThread m_thread;
    FlatpakBackendWorker* m_worker = nullptr;
};

#endif // FLATPAKBACKEND_H
