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

#ifndef DATABASE_H
#define DATABASE_H

#include <QList>
#include <QObject>
#include <QVariantList>
#include <glacierpackage.h>
#include <glacierpmbackend.h>

class DataBase : public QObject {
    Q_OBJECT
public:
    explicit DataBase(QObject* parent = nullptr);
    ~DataBase();

    Q_INVOKABLE void searchPackages(const QString& name);
    Q_INVOKABLE void getUpdates();
    Q_INVOKABLE void getOrphansPackages();
    Q_INVOKABLE void getPackage(const QString& name, const QString& repo = QString());
    Q_INVOKABLE QVariantMap findPackageByFile(const QString& filePath);
    Q_INVOKABLE void listRepositories();

    Q_INVOKABLE void install(const QString& packageName, const QString& repo = QString());
    Q_INVOKABLE void remove(const QString& packageName, const QString& repo = QString());
    Q_INVOKABLE void upgrade(const QString& packageName = QString(), const QString& repo = QString());
    Q_INVOKABLE void refresh();
    Q_INVOKABLE void addRepository(const QString& url, const QString& name, const QString& backend = QString());
    Q_INVOKABLE void removeRepository(const QString& id, const QString& backend = QString());
    Q_INVOKABLE void setRepositoryEnabled(const QString& id, bool enabled, const QString& backend = QString());

signals:
    void dbRefreshed();
    void searchPackagesReady(QVariantList packages);
    void getUpdatesReady(QVariantList packages);
    void getOrphansPackagesReady(QVariantList packages);
    void getPackageReady(GlacierPackage package);
    void listRepositoriesReady(QVariantList repositories);

    void transactionStarted();
    void transactionFinished();
    void authorizationFail();
    void emitAction(const QString& action);
    void emitActionProgress(const QString& action, const QString& status, double progress);
    void startWaiting();
    void stopWaiting();
    void startDownloading();
    void stopDownloading();
    void startPreparing();
    void stopPreparing();
    void showUpdatesRequested();

private:
    GlacierPMBackend* backendForRepo(const QString& repo) const;

    QList<GlacierPMBackend*> m_backends;
    int m_transactionActiveCount = 0;

    int m_searchGeneration = 0;
    int m_searchPending = 0;
    QVariantList m_searchResults;

    int m_updatesGeneration = 0;
    int m_updatesPending = 0;
    QVariantList m_updatesResults;

    int m_orphansGeneration = 0;
    int m_orphansPending = 0;
    QVariantList m_orphansResults;

    int m_packageGeneration = 0;
    int m_packagePending = 0;
    GlacierPackage m_packageResult;
    bool m_packageFound = false;

    int m_reposGeneration = 0;
    int m_reposPending = 0;
    QVariantList m_reposResults;
};

#endif // DATABASE_H
