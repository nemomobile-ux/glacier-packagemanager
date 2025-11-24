SET(PLUGINNAME empty)

SET(SRC ${PLUGINNAME}backend.cpp)

SET(HEADERS ${PLUGINNAME}backend.h)

set(CMAKE_AUTOMOC ON)
add_definitions(-DQT_PLUGIN)

add_library(${PLUGINNAME} MODULE ${SRC} ${HEADERS})
set_target_properties(${PLUGINNAME} PROPERTIES PREFIX "")

include_directories("../../")

target_link_libraries(${PLUGINNAME} PUBLIC
    Qt6::Core
    Glacier::PackageManager)

install(TARGETS ${PLUGINNAME}
    LIBRARY DESTINATION /usr/lib/glacier-pacmagemanager/backend/)
