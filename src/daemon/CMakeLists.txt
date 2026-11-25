set(SRC main.cpp)

qt_add_resources(DAEMON_RESOURCES daemon.qrc)
add_executable(glacier-packagemanager-daemon ${SRC} ${DAEMON_RESOURCES})

target_link_libraries(glacier-packagemanager-daemon
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick)

install(TARGETS glacier-packagemanager-daemon RUNTIME
    DESTINATION ${CMAKE_INSTALL_BINDIR})

install(FILES glacier-packagemanager-daemon.service
    DESTINATION /usr/lib/systemd/user)
install(FILES glacier-packagemanager-daemon.timer
    DESTINATION /usr/lib/systemd/user)
