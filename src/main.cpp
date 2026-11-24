/*
 * Copyright (C) 2018-2021 Chupligin Sergey <neochapay@gmail.com>
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

#ifdef QT_QML_DEBUG
#include <QtQuick>
#endif

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>

#include <cstdlib>

#include <glacierapp.h>

int main(int argc, char* argv[])
{
    setenv("QT_QUICK_CONTROLS_STYLE", "Nemo", 1);

    QGuiApplication* app = GlacierApp::app(argc, argv);
    app->setOrganizationName("NemoMobile");

    QQmlApplicationEngine* engine = GlacierApp::engine(app);
    QQmlContext* context = engine->rootContext();

    QCommandLineParser parser;
    parser.setApplicationDescription(QObject::tr("Glacier Package Manager"));
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption installOption(QStringList() << "i"
                                                   << "install",
        QCoreApplication::translate("main", "Install <package>."), QCoreApplication::translate("main", "package"));
    QCommandLineOption updateOption(QStringList() << "u"
                                                  << "update",
        QCoreApplication::translate("main", "Update packages."));
    QCommandLineOption removeOption(QStringList() << "r"
                                                  << "remove",
        QCoreApplication::translate("main", "Remove <package>"), QCoreApplication::translate("main", "package"));
    parser.addOption(installOption);
    parser.addOption(updateOption);
    parser.addOption(removeOption);

    parser.process(*app);

    if (parser.isSet(installOption)) {
        context->setContextProperty("action", QStringLiteral("install"));
        context->setContextProperty("pkgname", parser.value(installOption));
    } else if (parser.isSet(updateOption)) {
        context->setContextProperty("action", QStringLiteral("update"));
    } else if (parser.isSet(removeOption)) {
        context->setContextProperty("action", QStringLiteral("remove"));
        context->setContextProperty("pkgname", parser.value(removeOption));
    } else {
        context->setContextProperty("action", QString());
    }

    QQuickWindow* window = GlacierApp::showWindow();
    if (window) {
        window->setTitle(QObject::tr("Packages"));
        window->setIcon(QIcon(QStringLiteral("/usr/share/glacier-packagemanager/images/glacier-packagemanager.png")));
    }

    return app->exec();
}
