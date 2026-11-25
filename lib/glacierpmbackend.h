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

#ifndef GLACIERPMBACKEND_H
#define GLACIERPMBACKEND_H

#include "glacierpackage.h"
#include "glacierpm_global.h"
#include "glacierrepo.h"
#include <QMetaObject>
#include <QObject>
#include <QVariantList>

class GLACIERPM_EXPORT GlacierPMBackend : public QObject {
    Q_OBJECT

public:
    virtual QString backendId() const = 0;
    virtual bool enabled() = 0;
    virtual void install(QString packageName) = 0;
    virtual void installFromLocalFile(QString patch) = 0;
    virtual void remove(QString packageName) = 0;
    virtual void upgrade(QString packageName) = 0;
    virtual void update() = 0;

    virtual void searchPackages(const QString& name)
    {
        Q_UNUSED(name)
        QMetaObject::invokeMethod(this, [this]() { emit searchPackagesReady(QVariantList()); }, Qt::QueuedConnection);
    }
    virtual void getUpdates()
    {
        QMetaObject::invokeMethod(this, [this]() { emit getUpdatesReady(QVariantList()); }, Qt::QueuedConnection);
    }
    virtual void getOrphansPackages()
    {
        QMetaObject::invokeMethod(this, [this]() { emit getOrphansPackagesReady(QVariantList()); }, Qt::QueuedConnection);
    }
    virtual void getPackage(const QString& name)
    {
        Q_UNUSED(name)
        QMetaObject::invokeMethod(this, [this]() { emit getPackageReady(GlacierPackage()); }, Qt::QueuedConnection);
    }
    virtual void listRepositories()
    {
        QMetaObject::invokeMethod(this, [this]() { emit listRepositoriesReady(QVariantList()); }, Qt::QueuedConnection);
    }
    virtual void addRepository(const QString& url, const QString& name)
    {
        Q_UNUSED(url)
        Q_UNUSED(name)
    }
    virtual void removeRepository(const QString& id)
    {
        Q_UNUSED(id)
    }
    virtual void setRepositoryEnabled(const QString& id, bool enabled)
    {
        Q_UNUSED(id)
        Q_UNUSED(enabled)
    }
    virtual GlacierPackage findPackageByFile(const QString& filePath)
    {
        Q_UNUSED(filePath)
        return GlacierPackage();
    }

signals:
    void dbRefreshed();
    void searchPackagesReady(QVariantList packages);
    void getUpdatesReady(QVariantList packages);
    void getOrphansPackagesReady(QVariantList packages);
    void getPackageReady(GlacierPackage package);
    void listRepositoriesReady(QVariantList repositories);
    void emitAction(const QString& action);
    void emitActionProgress(const QString& action, const QString& status, double progress);
};

Q_DECLARE_INTERFACE(GlacierPMBackend, "GlacierPackageManager.Backend")

#endif // GLACIERPMBACKEND_H
