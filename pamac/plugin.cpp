/*
 * Copyright (C) 2021 Chupligin Sergey <neochapay@gmail.com>
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

#include "config.h"
#include "database.h"
#include "historyitemmodel.h"
#include "transaction.h"

#include <QQmlEngine>
#include <QQmlExtensionPlugin>
#include <QtGlobal>
#include <QtQml>

class Q_DECL_EXPORT GlacierPackageManagerPlugin : public QQmlExtensionPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.glacier.packagemanager")
public:
    virtual ~GlacierPackageManagerPlugin() { }

    void initializeEngine(QQmlEngine*, const char* uri)
    {
        Q_ASSERT(uri == QLatin1String("org.glacier.packagemanager"));
        qmlRegisterModule(uri, 1, 0);
    }

    void registerTypes(const char* uri)
    {
        Q_ASSERT(uri == QLatin1String("org.glacier.packagemanager"));
        qmlRegisterType<DataBase>(uri, 1, 0, "PackageDatabase");
        qmlRegisterType<Transaction>(uri, 1, 0, "PackageTransaction");
        qmlRegisterType<HistoryItemModel>(uri, 1, 0, "HistoryItemModel");
    }
};

#include "plugin.moc"
