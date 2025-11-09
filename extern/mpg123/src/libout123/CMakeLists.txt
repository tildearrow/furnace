cmake_minimum_required(VERSION 3.12)

include_directories("${CMAKE_CURRENT_BINARY_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libout123/")
add_subdirectory("modules")

set(TARGET libout123)
add_library(${TARGET}
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libout123/libout123.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libout123/stringlists.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libout123/wav.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libout123/hextxt.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libout123/$<$<NOT:$<BOOL:${NO_BUFFER}>>:buffer.c>"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libout123/$<$<NOT:$<BOOL:${NO_BUFFER}>>:xfermem.c>"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libout123/$<$<BOOL:${USE_MODULES}>:module.c>"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libout123/$<$<NOT:$<BOOL:${USE_MODULES}>>:legacy_module.c>"
    $<TARGET_OBJECTS:compat>
    $<$<BOOL:${USE_MODULES}>:$<TARGET_OBJECTS:compat_dl>>)

set_target_properties(${TARGET} PROPERTIES OUTPUT_NAME out123)

if(HAVE_UNIX_DL)
    string(APPEND LIBOUT123_LIBS " -ldl")
endif()
if(HAVE_M)
    string(APPEND LIBOUT123_LIBS " -lm")
endif()
if(HAVE_RT)
    string(APPEND LIBOUT123_LIBS " -lrt")
endif()
if(WANT_WIN32_UNICODE)
    string(APPEND LIBOUT123_LIBS " -lshlwapi")
endif()
set(LIBOUT123_LIBS "${LIBOUT123_LIBS}" PARENT_SCOPE)
target_link_libraries(${TARGET} PRIVATE
    $<TARGET_NAME_IF_EXISTS:defaultmodule>
    $<$<BOOL:${HAVE_UNIX_DL}>:dl>
    $<$<BOOL:${HAVE_M}>:m>
    $<$<BOOL:${HAVE_RT}>:rt>
    $<$<BOOL:${WANT_WIN32_UNICODE}>:shlwapi>)

target_include_directories(${TARGET} INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

install(TARGETS ${TARGET} EXPORT targets
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}/"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}/")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/include/out123.h"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
