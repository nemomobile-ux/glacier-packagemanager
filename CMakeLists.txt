cmake_minimum_required(VERSION 3.6.0)

project(glacier-packagemanager
	VERSION 0.1.1
	DESCRIPTION "The Glacier packagemanager")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include(FeatureSummary)
include(GNUInstallDirs)

set(QT_MIN_VERSION "5.11.0")
find_package(Qt6 COMPONENTS Gui Qml Quick LinguistTools REQUIRED)
find_package(Glacier 0.8.0 COMPONENTS App REQUIRED)
find_package(PkgConfig)

pkg_check_modules(PAMAC REQUIRED IMPORTED_TARGET pamac)
pkg_check_modules(GLIB2 REQUIRED IMPORTED_TARGET glib-2.0)
pkg_check_modules(GOBJECT2 REQUIRED IMPORTED_TARGET gobject-2.0)

add_subdirectory(src)
add_subdirectory(pamac)

# Translations
file(GLOB TS_FILES translations/*.ts)
qt_add_translation(QM_FILES ${TS_FILES})
add_custom_target(translations DEPENDS ${QM_FILES})
add_dependencies(glacier-packagemanager translations)

install(FILES ${QM_FILES}
	DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/glacier-packagemanager/translations)

install(FILES glacier-packagemanager.desktop
	DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications)
install(FILES glacier-packagemanager.png
	DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/glacier-packagemanager/images)
