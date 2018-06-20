
TARGET=glacier-packagemanager

qml.files = qml/
qml.path = /usr/share/glacier-packagemanager/

icon.files = glacier-packagemanager.png
icon.path = /usr/share/glacier-packagemanager/images/

CONFIG += link_pkgconfig
PKGCONFIG += glacierapp
PKGCONFIG += XPackageKit

INCLUDEPATH += /usr/include/XPackageKit/

QT += network dbus quick qml

SOURCES += src/main.cpp \
    src/packagemanager.cpp \
    src/models/updateslistmodel.cpp

target.path = $$INSTALL_ROOT/usr/bin/

desktop.files = glacier-packagemanager.desktop
desktop.path = $$INSTALL_ROOT/usr/share/applications

privileges.files = glacier-packagemanager.privileges
privileges.path = /usr/share/mapplauncherd/privileges.d/

INSTALLS += target desktop qml icon privileges

DISTFILES += \
    rpm/* \
    qml/main.qml \
    glacier-packagemanager.desktop \
    qml/glacier-packagemanager.qml \
    qml/pages/MainPage.qml \
    glacier-packagemanager.privileges \
    qml/pages/UpdatesPage.qml \
    qml/pages/RemovePackagePage.qml \
    qml/pages/InstallPackagePage.qml \
    README.md

HEADERS += \
    src/packagemanager.h \
    src/models/updateslistmodel.h
