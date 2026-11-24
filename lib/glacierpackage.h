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

#ifndef GLACIERPACKAGE_H
#define GLACIERPACKAGE_H

#include <QMetaType>
#include <QString>
#include <QVariantList>

struct GlacierPackage {
    Q_GADGET
    Q_PROPERTY(QString name MEMBER name)
    Q_PROPERTY(QString description MEMBER description)
    Q_PROPERTY(QString icon MEMBER icon)
    Q_PROPERTY(QString version MEMBER version)
    Q_PROPERTY(QString installedVersion MEMBER installedVersion)
    Q_PROPERTY(bool installed MEMBER installed)
    Q_PROPERTY(bool haveUpdates MEMBER haveUpdates)
    Q_PROPERTY(QString repo MEMBER repo)
    Q_PROPERTY(QVariantList screenshots MEMBER screenshots)

public:
    QString name;
    QString description;
    QString icon;
    QString version;
    QString installedVersion;
    bool installed = false;
    bool haveUpdates = false;
    QString repo;
    QVariantList screenshots;
};

Q_DECLARE_METATYPE(GlacierPackage)

#endif // GLACIERPACKAGE_H
