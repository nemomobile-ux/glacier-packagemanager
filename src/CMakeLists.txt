set(SRC
    models/updateslistmodel.h
    models/updateslistmodel.cpp
    pamac/database.cpp
    pamac/database.h
    pamac/config.cpp
    pamac/config.h
    pamac/updates.cpp
    pamac/updates.h
    packagemanager.h
    packagemanager.cpp
    main.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${XPACKAGEKIT_INCLUDE_DIRS})

add_executable(glacier-packagemanager ${SRC})

target_link_libraries(glacier-packagemanager
	Qt5::Gui
	Qt5::Qml
	Qt5::Quick
	Glacier::App
	PkgConfig::XPACKAGEKIT
        PkgConfig::PAMAC
        PkgConfig::GLIB2
        PkgConfig::GOBJECT2)

install(TARGETS glacier-packagemanager RUNTIME
	DESTINATION ${CMAKE_INSTALL_BINDIR})

add_definitions(-DQT_NO_KEYWORDS)
