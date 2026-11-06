cmake_minimum_required(VERSION 3.28)

add_library(rtmidi_modules)

set(RTMIDI_MODULES
    RtMidi.cppm
)

if(NOT COMMAND configure_cpp_module_target)
    function(configure_cpp_module_target target)
        target_sources(${target} PUBLIC FILE_SET CXX_MODULES FILES ${RTMIDI_MODULES})
    endfunction()
endif()

if(RTMIDI_USE_NAMESPACE)
    target_compile_definitions(rtmidi_modules PRIVATE RTMIDI_USE_NAMESPACE)
endif()

configure_cpp_module_target(rtmidi_modules)

target_link_libraries(rtmidi_modules
    PUBLIC
    rtmidi
)

target_include_directories(rtmidi_modules
    PRIVATE
    ${PROJECT_SOURCE_DIR}
)

target_compile_features(rtmidi_modules PUBLIC cxx_std_20)