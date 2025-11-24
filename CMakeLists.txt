cmake_minimum_required(VERSION 3.10.0)

project(glacier-packagemanager
        VERSION 0.2.0
	DESCRIPTION "The Glacier packagemanager")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INSTALL_PREFIX /usr)

set(PACKAGE_BACKEND "empty")

include(FeatureSummary)
include(GNUInstallDirs)

set(QT_MIN_VERSION "6.0.0")
find_package(Qt6 COMPONENTS Gui Qml Quick LinguistTools REQUIRED)
find_package(Glacier 0.8.0 COMPONENTS App REQUIRED)
find_package(PkgConfig)

add_subdirectory(lib)
add_subdirectory(src)
add_subdirectory(plugin)
#add_subdirectory(pamac)

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
