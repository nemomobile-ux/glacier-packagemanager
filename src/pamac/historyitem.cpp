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


#include "historyitem.h"

#include <QRegularExpression>

HistoryItem::~HistoryItem()
{

}

QString HistoryItem::typeToString(PackageType t) const{
    switch (t) {
    case Installed:
        return "Installed";
    case Removed:
        return "Removed";
    case Upgraded:
        return "Upgraded";
    default:
        return "Unknown";
    }
}

HistoryItem::PackageType HistoryItem::typeFromString(const QString &s){
    QString lowS = s.toLower();
    if(lowS=="installed"){
        return Installed;
    }
    if(lowS=="removed"){
        return Removed;
    }
    if(lowS=="upgraded"){
        return Upgraded;
    }
    return Unknown;
}

void HistoryItem::setType(const QString &type){
    m_type = typeFromString(type);
}

QList<HistoryItem> HistoryItem::fromStringList(const QStringList &list)
{
    QList<HistoryItem> result;
    const QRegularExpression exp("\\[(.+)\\] \\[ALPM\\] (\\S+) (\\S+) \\((.+)\\)");
    for(const QString& el:list){
        QRegularExpressionMatch match;
        if((match = exp.match(el)).hasMatch()){
            HistoryItem item;
            item.m_time = QDateTime::fromString(match.captured(1),Qt::ISODate);
            item.m_type = typeFromString(match.captured(2));
            item.m_name = match.captured(3);
            item.m_version = match.captured(4);

            result.append(item);
        }
    }
    return result;
}
