SET(PLUGIN packagemanager)

SET(SRC ${PLUGIN}settingsplugin.cpp)
SET(HEADERS ${PLUGIN}settingsplugin.h)

SET(PLUGINS_FILES
    ${PLUGIN}.qml
    ${PLUGIN}.svg)

set(CMAKE_AUTOMOC ON)

add_definitions(-DQT_PLUGIN)

add_library(${PLUGIN}settings MODULE ${SRC} ${HEADERS})
target_include_directories(${PLUGIN}settings PUBLIC "/usr/include/glacier-settings/")

target_link_libraries(${PLUGIN}settings
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    -lglaciersettings)

install(TARGETS ${PLUGIN}settings
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/glacier-settings/)

install(FILES ${PLUGINS_FILES}
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/glacier-settings/plugins/${PLUGIN}/)

# Translations
file(GLOB TS_FILES translations/*.ts)
qt6_add_translation(QM_FILES ${TS_FILES})
add_custom_target(packagemanager_settings_translations DEPENDS ${QM_FILES})
add_dependencies(${PLUGIN}settings packagemanager_settings_translations)

install(FILES ${QM_FILES}
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/glacier-settings/translations)
