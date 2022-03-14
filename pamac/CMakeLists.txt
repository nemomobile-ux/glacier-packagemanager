### Sets QT_INSTALL_QML to the directory where QML Plugins should be installed
function(FindQtInstallQml)
    find_program(QMAKE NAMES qmake-qt5 qmake)
    if(NOT QMAKE)
    message(FATAL_ERROR "qmake not found")
    endif()
    execute_process(
        COMMAND ${QMAKE} -query QT_INSTALL_QML
        OUTPUT_VARIABLE PROC_RESULT
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(QT_INSTALL_QML ${PROC_RESULT} PARENT_SCOPE)
endfunction()

add_definitions(-DQT_NO_KEYWORDS)

set(SRC
    plugin.cpp
    database.cpp
    config.cpp
    updates.cpp
    transaction.cpp
    historyitem.cpp
    historyitemmodel.cpp)

set(HEADERS
    database.h
    config.h
    updates.h
    transaction.h
    historyitem.h
    historyitemmodel.h)

add_library(glacierpackagemanager SHARED ${SRC})

target_link_libraries(glacierpackagemanager
    Qt5::Core
    Qt5::Qml
    Qt5::Quick
    PkgConfig::PAMAC
    PkgConfig::GLIB2
    PkgConfig::GOBJECT2)

FindQtInstallQml()

install(FILES qmldir DESTINATION ${QT_INSTALL_QML}/org/glacier/packagemanager/)
install(TARGETS glacierpackagemanager DESTINATION ${QT_INSTALL_QML}/org/glacier/packagemanager/)
