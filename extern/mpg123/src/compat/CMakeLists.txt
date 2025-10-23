set(TARGET compat)
add_library(${TARGET} OBJECT
    "compat.c"
    "compat_str.c")
if(BUILD_SHARED_LIBS)
    set_target_properties(${TARGET} PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

add_library(${TARGET}_dl OBJECT
    "compat_dl.c")
if(BUILD_SHARED_LIBS)
    set_target_properties(${TARGET}_dl PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

add_library(${TARGET}_str OBJECT
    "compat_str.c")
if(BUILD_SHARED_LIBS)
    set_target_properties(${TARGET}_str PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()
