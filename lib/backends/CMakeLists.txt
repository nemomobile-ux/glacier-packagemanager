option(ENABLE_APT_BACKEND "Build APT package manager backend" OFF)

add_subdirectory(empty)

if(ENABLE_APT_BACKEND)
    add_subdirectory(apt)
endif()
