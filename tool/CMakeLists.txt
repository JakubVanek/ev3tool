set(TOOL_VERSION_MAJOR 0)
set(TOOL_VERSION_MINOR 1)
set(TOOL_VERSION_PATCH 0)
set(TOOL_VERSION "${TOOL_VERSION_MAJOR}.${TOOL_VERSION_MINOR}.${TOOL_VERSION_PATCH}")
message("ev3tool ${TOOL_VERSION}")

if (USE_GETTEXT)
    # Process translations
    FILE(READ "${CMAKE_CURRENT_SOURCE_DIR}/po/LINGUAS" langs_string)
    STRING(REGEX MATCHALL "[a-zA-Z_]+" langs "${langs_string}")
    FOREACH(lang ${langs})
        GETTEXT_PROCESS_PO_FILES("${lang}" ALL INSTALL_DESTINATION "${LOCALEDIR}"
                PO_FILES "${CMAKE_CURRENT_SOURCE_DIR}/po/${lang}/ev3tool.po")
    ENDFOREACH()
endif()

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/config.in.hpp"
               "${CMAKE_CURRENT_SOURCE_DIR}/src/config.hpp")

file(GLOB_RECURSE PRJ_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE PRJ_HDR ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp)

# Artifacts
add_executable(ev3tool ${PRJ_SRC} ${PRJ_HDR})
target_include_directories(ev3tool PUBLIC "${CMAKE_SOURCE_DIR}/library/src" ${Intl_INCLUDE_DIRS} ${HIDAPI_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS})
target_link_libraries(ev3tool ev3comm stdc++fs ${Intl_LIBRARIES} ${HIDAPI_LIBRARIES} ${ZLIB_LIBRARIES})

install(TARGETS ev3tool
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
