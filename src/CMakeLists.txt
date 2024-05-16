set(SRC main.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${XPACKAGEKIT_INCLUDE_DIRS})

qt_add_resources(RESOURCES qml/glacier-pacmagemanager.qrc)
add_executable(glacier-packagemanager ${SRC} ${RESOURCES})

target_link_libraries(glacier-packagemanager
	Qt6::Gui
	Qt6::Qml
	Qt6::Quick
        Glacier::App)

install(TARGETS glacier-packagemanager RUNTIME
	DESTINATION ${CMAKE_INSTALL_BINDIR})

add_definitions(-DQT_NO_KEYWORDS)
