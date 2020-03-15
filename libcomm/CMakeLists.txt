set(LIB_VERSION_MAJOR 0)
set(LIB_VERSION_MINOR 3)
set(LIB_VERSION_PATCH 0)
set(LIB_VERSION "${LIB_VERSION_MAJOR}.${LIB_VERSION_MINOR}.${LIB_VERSION_PATCH}")
message("libev3comm ${LIB_VERSION}")

if (USE_GETTEXT)
    # Process translations
    FILE(READ "${CMAKE_CURRENT_SOURCE_DIR}/po/LINGUAS" langs_string)
    STRING(REGEX MATCHALL "[a-zA-Z_]+" langs "${langs_string}")
    FOREACH (lang ${langs})
        GETTEXT_PROCESS_PO_FILES("${lang}" ALL INSTALL_DESTINATION "${LOCALEDIR}"
                PO_FILES "${CMAKE_CURRENT_SOURCE_DIR}/po/${lang}/libev3comm.po")
    ENDFOREACH ()
endif ()

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config.in.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/include/ev3loader/config.hpp")

file(GLOB_RECURSE PRJ_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE PRJ_HDR ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp)

# Artifacts
add_library(ev3comm SHARED ${PRJ_SRC} ${PRJ_HDR})
target_include_directories(ev3comm
        PUBLIC
        ${Intl_INCLUDE_DIRS}
        ${HIDAPI_INCLUDE_DIRS}
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src)


target_link_libraries(ev3comm stdc++fs ${Intl_LIBRARIES} ${HIDAPI_LIBRARIES})

install(TARGETS ev3comm
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
