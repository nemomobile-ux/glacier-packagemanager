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

#ifndef GLACIERREPO_H
#define GLACIERREPO_H

#include <QMetaType>
#include <QString>

struct GlacierRepo {
    Q_GADGET
    Q_PROPERTY(QString id MEMBER id)
    Q_PROPERTY(QString name MEMBER name)
    Q_PROPERTY(QString url MEMBER url)
    Q_PROPERTY(QString backend MEMBER backend)
    Q_PROPERTY(bool enabled MEMBER enabled)

public:
    QString id;
    QString name;
    QString url;
    QString backend;
    bool enabled = true;
};

Q_DECLARE_METATYPE(GlacierRepo)

#endif // GLACIERREPO_H
