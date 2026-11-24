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
    Q_INVOKABLE void getPackage(const QString& name);

    Q_INVOKABLE void install(const QString& packageName);
    Q_INVOKABLE void remove(const QString& packageName);
    Q_INVOKABLE void upgrade(const QString& packageName = QString());
    Q_INVOKABLE void refresh();

signals:
    void dbRefreshed();
    void searchPackagesReady(QVariantList packages);
    void getUpdatesReady(QVariantList packages);
    void getOrphansPackagesReady(QVariantList packages);
    void getPackageReady(GlacierPackage package);

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

private:
    GlacierPMBackend* m_backend = nullptr;
    bool m_transactionActive = false;
};

#endif // DATABASE_H
