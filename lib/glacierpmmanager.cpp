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

#include "glacierpmmanager.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QProcessEnvironment>

GlacierPMManager::GlacierPMManager(QObject* parent)
    : QObject { parent }
{
    QString backendDir = QStringLiteral("/usr/lib/glacier-packagemanager/backend");
    const QString envBackendDir = QProcessEnvironment::systemEnvironment().value(QStringLiteral("GLACIER_PM_BACKEND_DIR"));
    if (!envBackendDir.isEmpty()) {
        backendDir = envBackendDir;
    }
    const QStringList candidates = {
        backendDir + QStringLiteral("/apt.so"),
        backendDir + QStringLiteral("/rpm.so"),
        backendDir + QStringLiteral("/snap.so")
    };

    for (const QString& candidate : candidates) {
        if (!QFileInfo::exists(candidate)) {
            continue;
        }

        QPluginLoader* loader = new QPluginLoader(candidate, this);
        QObject* plugin = loader->instance();
        if (!plugin) {
            qWarning() << "Cannot load backend plugin file:" << candidate << loader->errorString();
            loader->unload();
            delete loader;
            continue;
        }

        GlacierPMBackend* backend = qobject_cast<GlacierPMBackend*>(plugin);
        if (!backend) {
            qWarning() << "Cannot cast backend plugin:" << candidate;
            loader->unload();
            delete loader;
            continue;
        }

        if (!backend->enabled()) {
            qWarning() << "Backend plugin disabled:" << candidate;
            loader->unload();
            delete loader;
            continue;
        }

        m_backends.append(backend);
        m_pluginLoaders.append(loader);
        qDebug() << "Loaded package manager backend:" << candidate;
    }

    if (m_backends.isEmpty()) {
        const QString emptyCandidate = backendDir + QStringLiteral("/empty.so");
        if (QFileInfo::exists(emptyCandidate)) {
            QPluginLoader* loader = new QPluginLoader(emptyCandidate, this);
            QObject* plugin = loader->instance();
            GlacierPMBackend* backend = qobject_cast<GlacierPMBackend*>(plugin);
            if (backend && backend->enabled()) {
                m_backends.append(backend);
                m_pluginLoaders.append(loader);
                qDebug() << "Loaded package manager backend:" << emptyCandidate;
            } else {
                if (!plugin) {
                    qWarning() << "Cannot load backend plugin file:" << emptyCandidate << loader->errorString();
                }
                loader->unload();
                delete loader;
            }
        }
    }

    if (m_backends.isEmpty()) {
        qWarning() << "No package manager backend found in" << backendDir;
    }
}

GlacierPMManager::~GlacierPMManager()
{
    m_backends.clear();
    for (QPluginLoader* loader : m_pluginLoaders) {
        loader->unload();
        delete loader;
    }
    m_pluginLoaders.clear();
}

QList<GlacierPMBackend*> GlacierPMManager::backends() const
{
    return m_backends;
}
