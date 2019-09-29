set(LIB_VERSION_MAJOR 0)
set(LIB_VERSION_MINOR 2)
set(LIB_VERSION_PATCH 0)
set(LIB_VERSION "${LIB_VERSION_MAJOR}.${LIB_VERSION_MINOR}.${LIB_VERSION_PATCH}")
message("libev3asm ${LIB_VERSION}")

if (USE_GETTEXT)
    # Process translations
    FILE(READ "${CMAKE_CURRENT_SOURCE_DIR}/po/LINGUAS" langs_string)
    STRING(REGEX MATCHALL "[a-zA-Z_]+" langs "${langs_string}")
    FOREACH(lang ${langs})
        GETTEXT_PROCESS_PO_FILES("${lang}" ALL INSTALL_DESTINATION "${LOCALEDIR}"
                PO_FILES "${CMAKE_CURRENT_SOURCE_DIR}/po/${lang}/libev3asm.po")
    ENDFOREACH()
endif()

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/config.in.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/config.hpp")

file(GLOB_RECURSE PRJ_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE PRJ_HDR ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp)

# Artifacts
add_library(ev3asm SHARED ${PRJ_SRC} ${PRJ_HDR})
target_include_directories(ev3asm PUBLIC ev3comm ${Intl_INCLUDE_DIRS})
target_link_libraries(ev3asm ev3comm stdc++fs ${Intl_LIBRARIES})

install(TARGETS ev3asm
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)