set(PLUGINNAME apt)

find_package(PkgConfig REQUIRED)
pkg_check_modules(APT_PKG REQUIRED IMPORTED_TARGET apt-pkg)
find_package(AppStreamQt 1.0 QUIET)

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
    PkgConfig::APT_PKG)

if(AppStreamQt_FOUND)
    target_link_libraries(${PLUGINNAME} PUBLIC AppStreamQt)
    target_compile_definitions(${PLUGINNAME} PRIVATE HAVE_APPSTREAMQT)
    message(STATUS "APT backend: AppStreamQt support enabled")
else()
    message(STATUS "APT backend: AppStreamQt support disabled")
endif()

install(TARGETS ${PLUGINNAME}
    LIBRARY DESTINATION /usr/lib/glacier-packagemanager/backend/)

add_executable(glacier-packagemanager-helper apthelper.cpp)
target_link_libraries(glacier-packagemanager-helper PRIVATE
    PkgConfig::APT_PKG)

install(TARGETS glacier-packagemanager-helper
    RUNTIME DESTINATION ${CMAKE_INSTALL_LIBEXECDIR})
