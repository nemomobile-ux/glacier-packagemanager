set(SRC main.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${XPACKAGEKIT_INCLUDE_DIRS})

qtquick_compiler_add_resources(RESOURCES qml/glacier-pacmagemanager.qrc)
add_executable(glacier-packagemanager ${SRC} ${RESOURCES})

target_link_libraries(glacier-packagemanager
	Qt5::Gui
	Qt5::Qml
	Qt5::Quick
        Glacier::App)

install(TARGETS glacier-packagemanager RUNTIME
	DESTINATION ${CMAKE_INSTALL_BINDIR})

add_definitions(-DQT_NO_KEYWORDS)
