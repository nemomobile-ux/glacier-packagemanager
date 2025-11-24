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

#ifndef EMPTYBACKEND_H
#define EMPTYBACKEND_H

#include <QObject>
#include <glacierpmbackend.h>

class EmptyBackend : public GlacierPMBackend {
    Q_OBJECT
    Q_INTERFACES(GlacierPMBackend)
    Q_PLUGIN_METADATA(IID "GlacierPackageManager.Backend")

public:
    EmptyBackend();
    bool enabled() { return true; }
    void install(QString packageName);
    void installFromLocalFile(QString patch);
    void remove(QString packageName);
    void upgrade(QString packageName);
    void update();
};

#endif // EMPTY_H
