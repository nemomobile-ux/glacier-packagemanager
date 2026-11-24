set(SRC main.cpp)

qt_add_resources(RESOURCES qml/glacier-packagemanager.qrc)
add_executable(glacier-packagemanager ${SRC} ${RESOURCES})

target_link_libraries(glacier-packagemanager
	Qt6::Gui
	Qt6::Qml
	Qt6::Quick
	Glacier::App)

install(TARGETS glacier-packagemanager RUNTIME
	DESTINATION ${CMAKE_INSTALL_BINDIR})

add_definitions(-DQT_NO_KEYWORDS)
