option(ENABLE_APT_BACKEND "Build APT package manager backend" OFF)
option(ENABLE_RPM_BACKEND "Build RPM package manager backend" OFF)
option(ENABLE_SNAP_BACKEND "Build Snap package manager backend" OFF)

if(ENABLE_APT_BACKEND AND ENABLE_RPM_BACKEND)
    message(FATAL_ERROR "ENABLE_APT_BACKEND and ENABLE_RPM_BACKEND cannot be enabled at the same time")
endif()

add_subdirectory(empty)

if(ENABLE_APT_BACKEND)
    add_subdirectory(apt)
endif()

if(ENABLE_RPM_BACKEND)
    add_subdirectory(rpm)
endif()

if(ENABLE_SNAP_BACKEND)
    add_subdirectory(snap)
endif()
