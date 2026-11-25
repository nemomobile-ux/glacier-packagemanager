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

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QTranslator>

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);
    app.setOrganizationName(QStringLiteral("NemoMobile"));
    app.setApplicationName(QStringLiteral("glacier-packagemanager-daemon"));

    QTranslator myappTranslator;
    if (myappTranslator.load(QLocale(), QLatin1String("glacier-packagemanager"), QLatin1String("_"), QLatin1String("/usr/share/glacier-packagemanager/translations/"))) {
        app.installTranslator(&myappTranslator);
    }

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/daemon.qml")));

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}
