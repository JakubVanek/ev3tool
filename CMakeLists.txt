# CMake stuff
cmake_minimum_required(VERSION 3.5)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Project name and version
project("EV3comm")

# C++ standard
set(CMAKE_CXX_STANDARD 14)

# Build target
if (NOT CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE Release CACHE STRING
            "Choose the type of build, options are: Debug, Release, RelWithDebInfo, MinSizeRel."
            FORCE)
endif ()
message("Build type: ${CMAKE_BUILD_TYPE}")


option(SINGLEDIR "Store everything relative to the executable" ON)
set(LOCALEDIR "${CMAKE_INSTALL_PREFIX}/share/locale" CACHE PATH "Locale directory")

option(USE_GETTEXT "Use gettext for localization" ON)
if (USE_GETTEXT)
    # Localization
    find_package(Intl)
    find_package(Gettext)
    if(NOT Intl_FOUND)
        message(FATAL_ERROR "Cannot find libintl.")
    endif()
    if(NOT GETTEXT_FOUND)
        message(FATAL_ERROR "Cannot find gettext.")
    endif()
else()
    set(Intl_INCLUDE_DIRS "")
    set(Intl_LIBRARIES "")
endif()

find_package(HIDAPI)
if(NOT HIDAPI_FOUND)
    message(FATAL_ERROR "Cannot find HIDAPI.")
endif()

find_package(ZLIB)
if(NOT ZLIB_FOUND)
    message(FATAL_ERROR "Cannot find zlib.")
endif()

SET(CMAKE_SKIP_BUILD_RPATH  FALSE)
SET(CMAKE_INSTALL_RPATH "\$ORIGIN/../lib")
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)

add_subdirectory(libcomm)
add_subdirectory(libasm)
add_subdirectory(tool)
