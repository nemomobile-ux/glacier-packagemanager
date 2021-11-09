set(SRC
    models/updateslistmodel.h
    models/updateslistmodel.cpp
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
	)

install(TARGETS glacier-packagemanager RUNTIME
	DESTINATION ${CMAKE_INSTALL_BINDIR})
