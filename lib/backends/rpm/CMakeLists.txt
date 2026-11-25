set(PLUGINNAME rpm)

    find_package(PkgConfig REQUIRED)
        pkg_check_modules(LIBZYPP REQUIRED IMPORTED_TARGET libzypp)
            find_package(AppStreamQt 1.0 QUIET)

                set(SRC
                        $ { PLUGINNAME } backend.cpp
                            $ { PLUGINNAME } backendworker.cpp)

                    set(HEADERS
                            $ { PLUGINNAME } backend.h
                                $ { PLUGINNAME } backendworker.h)

                        set(CMAKE_AUTOMOC ON)
                            add_definitions(-DQT_PLUGIN)

                                add_library($ { PLUGINNAME } MODULE $ { SRC } $ { HEADERS })
                                    set_target_properties($ { PLUGINNAME } PROPERTIES PREFIX "")

                                        target_include_directories($ { PLUGINNAME } PRIVATE "../../")

                                            target_link_libraries($ { PLUGINNAME } PUBLIC
                                                    Qt6::Core
                                                        Glacier::PackageManager
                                                            PkgConfig::LIBZYPP
                                                                rpm
                                                                    rpmio)

                                                if (AppStreamQt_FOUND)
                                                    target_link_libraries($ { PLUGINNAME } PUBLIC AppStreamQt)
                                                        target_compile_definitions($ { PLUGINNAME } PRIVATE HAVE_APPSTREAMQT)
                                                            message(STATUS "RPM backend: AppStreamQt support enabled") else()
                                                                message(STATUS "RPM backend: AppStreamQt support disabled")
                                                                    endif()

                                                                        install(TARGETS $ { PLUGINNAME } LIBRARY DESTINATION / usr / lib / glacier - packagemanager / backend /)

                                                                            add_executable(glacier - packagemanager - rpm - helper rpmhelper.cpp)
                                                                                set_target_properties(glacier - packagemanager - rpm - helper PROPERTIES OUTPUT_NAME glacier - packagemanager - helper)
                                                                                    target_link_libraries(glacier - packagemanager - rpm - helper PRIVATE PkgConfig::LIBZYPP)

                                                                                        install(TARGETS glacier - packagemanager - rpm - helper RUNTIME DESTINATION $ { CMAKE_INSTALL_LIBEXECDIR })
