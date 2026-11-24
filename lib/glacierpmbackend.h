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

#include "glacierpm_global.h"
#include "glacierpackage.h"
#include <QMetaObject>
#include <QObject>
#include <QVariantList>

class GLACIERPM_EXPORT GlacierPMBackend : public QObject {
    Q_OBJECT

public:
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

signals:
    void dbRefreshed();
    void searchPackagesReady(QVariantList packages);
    void getUpdatesReady(QVariantList packages);
    void getOrphansPackagesReady(QVariantList packages);
    void getPackageReady(GlacierPackage package);
    void emitAction(const QString& action);
    void emitActionProgress(const QString& action, const QString& status, double progress);
};

Q_DECLARE_INTERFACE(GlacierPMBackend, "GlacierPackageManager.Backend")

#endif // GLACIERPMBACKEND_H
