set(SRC glacierpmmanager.cpp)

set(HEADERS glacierpmmanager.h)

set(PUBLIC_HEADERS
    glacierpmbackend.h
    glacierpm_global.h)

add_library(glacierpmmamager SHARED ${SRC} ${HEADERS} ${PUBLIC_HEADERS})
add_library(Glacier::PackageManager ALIAS glacierpmmamager)

add_subdirectory(backends)

target_link_libraries(glacierpmmamager
    Qt6::Core)

install(TARGETS glacierpmmamager
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/)

install(FILES ${PUBLIC_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/glacier-packagemanager)
