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

#ifndef HISTORYITEM_H
#define HISTORYITEM_H

#include <QDateTime>
#include <QObject>
#include <QVariant>

class HistoryItem {
    Q_GADGET
    Q_PROPERTY(QString name READ name)
    Q_PROPERTY(QString version READ version)
    Q_PROPERTY(QDateTime time READ time)
    Q_PROPERTY(PackageType type READ type)

public:
    ~HistoryItem();
    HistoryItem() = default;
    HistoryItem(const HistoryItem& other) = default;
    HistoryItem& operator=(const HistoryItem& other) = default;

    enum PackageType {
        Installed,
        Removed,
        Upgraded,
        Unknown
    };
    Q_ENUM(PackageType)

    QString name() const { return m_name; }
    QString version() const { return m_version; }
    const QDateTime time() const { return m_time; }
    PackageType type() const { return m_type; }

    QString typeToString(PackageType t) const;
    PackageType typeFromString(const QString& s);
    void setType(const QString& type);
    QList<HistoryItem> fromStringList(const QStringList& list);

private:
    QDateTime m_time;
    PackageType m_type;
    QString m_name;
    QString m_version;
};

#endif // HISTORYITEM_H
