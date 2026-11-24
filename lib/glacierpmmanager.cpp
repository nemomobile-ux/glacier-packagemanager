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
    , m_backend(nullptr)
    , m_pluginLoader(this)
{
    QString backendDir = QStringLiteral("/usr/lib/glacier-packagemanager/backend");
    const QString envBackendDir = QProcessEnvironment::systemEnvironment().value(QStringLiteral("GLACIER_PM_BACKEND_DIR"));
    if (!envBackendDir.isEmpty()) {
        backendDir = envBackendDir;
    }
    const QStringList candidates = {
        backendDir + QStringLiteral("/apt.so"),
        backendDir + QStringLiteral("/rpm.so"),
        backendDir + QStringLiteral("/empty.so")
    };

    for (const QString& candidate : candidates) {
        if (!QFileInfo::exists(candidate)) {
            continue;
        }

        m_pluginLoader.setFileName(candidate);
        QObject* plugin = m_pluginLoader.instance();
        if (!plugin) {
            qWarning() << "Cannot load backend plugin file:" << candidate << m_pluginLoader.errorString();
            m_pluginLoader.unload();
            continue;
        }

        GlacierPMBackend* backend = qobject_cast<GlacierPMBackend*>(plugin);
        if (!backend) {
            qWarning() << "Cannot cast backend plugin:" << candidate;
            m_pluginLoader.unload();
            continue;
        }

        if (!backend->enabled()) {
            qWarning() << "Backend plugin disabled:" << candidate;
            m_pluginLoader.unload();
            continue;
        }

        m_backend = backend;
        qDebug() << "Loaded package manager backend:" << candidate;
        return;
    }

    qWarning() << "No package manager backend found in" << backendDir;
}

GlacierPMManager::~GlacierPMManager() = default;

GlacierPMBackend* GlacierPMManager::getBackend()
{
    return m_backend;
}
