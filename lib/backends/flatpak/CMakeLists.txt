set(PLUGINNAME flatpak)

find_package(PkgConfig REQUIRED)
pkg_check_modules(FLATPAK REQUIRED IMPORTED_TARGET flatpak)

set(SRC
    ${PLUGINNAME}backend.cpp
    ${PLUGINNAME}backendworker.cpp)

set(HEADERS
    ${PLUGINNAME}backend.h
    ${PLUGINNAME}backendworker.h)

set(CMAKE_AUTOMOC ON)
add_definitions(-DQT_PLUGIN)

add_library(${PLUGINNAME} MODULE ${SRC} ${HEADERS})
set_target_properties(${PLUGINNAME} PROPERTIES PREFIX "")

target_include_directories(${PLUGINNAME} PRIVATE "../../")

target_link_libraries(${PLUGINNAME} PUBLIC
    Qt6::Core
    Glacier::PackageManager
    PkgConfig::FLATPAK)

install(TARGETS ${PLUGINNAME}
    LIBRARY DESTINATION /usr/lib/glacier-packagemanager/backend/)
