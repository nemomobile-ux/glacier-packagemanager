### Sets QT_INSTALL_QML to the directory where QML Plugins should be installed
function(FindQtInstallQml)
    find_program(QMAKE NAMES qmake6)
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

set(SRC
    plugin.cpp
    database.cpp)

set(HEADERS
    database.h)

include_directories("../lib")

add_library(glacierpackagemanager SHARED ${SRC})

qt6_add_qml_module(glacierpackagemanager
    URI Glacier.PackageManager
    VERSION 2.0
    PLUGIN_TARGET glacierpackagemanager
    NO_GENERATE_PLUGIN_SOURCE
    SOURCES
        ${SOURCES}
        ${HEADERS}
    QML_FILES
        ${QML}
    OUTPUT_DIRECTORY
        ${CMAKE_BINARY_DIR}/Glacier/PackageManager
        RESOURCES qmldir
)

target_link_libraries(glacierpackagemanager PUBLIC
    Qt6::Core
    Qt6::Qml
    Qt6::Quick
    Glacier::PackageManager)

FindQtInstallQml()

install(FILES qmldir DESTINATION ${QT_INSTALL_QML}/Glacier/PackageManager/)
install(TARGETS glacierpackagemanager DESTINATION ${QT_INSTALL_QML}/Glacier/PackageManager/)
