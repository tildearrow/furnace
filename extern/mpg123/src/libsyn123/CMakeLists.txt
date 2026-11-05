cmake_minimum_required(VERSION 3.12)

include_directories("${CMAKE_CURRENT_BINARY_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/libsyn123/")

set(TARGET libsyn123)
add_library(${TARGET}
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libsyn123/pinknoise.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libsyn123/geiger.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libsyn123/libsyn123.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libsyn123/volume.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libsyn123/resample.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libsyn123/filter.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/libsyn123/sampleconv.c"
    $<TARGET_OBJECTS:compat_str>)

set_target_properties(${TARGET} PROPERTIES OUTPUT_NAME syn123)

target_include_directories(${TARGET} INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

if(HAVE_M)
    string(APPEND LIBSYN123_LIBS " -lm")
endif()
set(LIBSYN123_LIBS "${LIBSYN123_LIBS}" PARENT_SCOPE)
target_link_libraries(${TARGET} PRIVATE
    $<$<BOOL:${HAVE_M}>:m>)

install(TARGETS ${TARGET} EXPORT targets
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}/"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}/")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/include/syn123.h"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
