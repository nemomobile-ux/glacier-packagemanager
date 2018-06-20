/*
 * Copyright (C) 2018 Chupligin Sergey <neochapay@gmail.com>
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
#include <QtGui/QGuiApplication>


#include <QtQml>
#include <QtQuick/QQuickView>
#include <QtCore/QString>
#include <QScreen>
#include <QCoreApplication>

#include <glacierapp.h>

#include "packagemanager.h"
#include "models/updateslistmodel.h"

int main(int argc, char *argv[])
{
    qmlRegisterType<PackageManager>("org.glacier.packagemanager",1,0,"PackageManager");
    qmlRegisterType<UpdatesListModel>("org.glacier.packagemanager",1,0,"UpdatesListModel");

    QGuiApplication *app = GlacierApp::app(argc, argv);
    app->setOrganizationName("NemoMobile");

    QQmlApplicationEngine *engine = GlacierApp::engine(app);
    QQmlContext *context = engine->rootContext();

    QCommandLineParser parser;
    parser.setApplicationDescription("Test helper");
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption installOption(QStringList() << "i" << "install",  QCoreApplication::translate("main", "Install <package>."), QCoreApplication::translate("main", "package"));
    QCommandLineOption updateOption(QStringList() << "u" << "update",  QCoreApplication::translate("main", "Update packages."));
    QCommandLineOption removeOption(QStringList() << "r" << "remove",  QCoreApplication::translate("main", "Remove <package>"), QCoreApplication::translate("main", "package"));
    parser.addOption(installOption);
    parser.addOption(updateOption);
    parser.addOption(removeOption);

    parser.process(*app);

    if(parser.isSet(installOption)) {
        context->setContextProperty("action", "install");
        context->setContextProperty("pkgname", parser.value(installOption));
    } else if(parser.isSet(updateOption)) {
        context->setContextProperty("action", "update");
    } else if(parser.isSet(removeOption)) {
        context->setContextProperty("action", "remove");
        context->setContextProperty("pkgname", parser.value(removeOption));
    } else {
        context->setContextProperty("action", "");
    }

    QQuickWindow *window = GlacierApp::showWindow();
    window->setTitle(QObject::tr("Package manager"));

    return app->exec();
}
