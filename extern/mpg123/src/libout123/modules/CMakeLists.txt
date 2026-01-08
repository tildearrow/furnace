if(NOT USE_MODULES)
    add_library(defaultmodule STATIC
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../src/libout123/modules/$<$<STREQUAL:${DEFAULT_OUTPUT_MODULE},dummy>:dummy.c>"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../src/libout123/modules/$<$<STREQUAL:${DEFAULT_OUTPUT_MODULE},coreaudio>:coreaudio.c>"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../src/libout123/modules/$<$<STREQUAL:${DEFAULT_OUTPUT_MODULE},alsa>:alsa.c>"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../src/libout123/modules/$<$<STREQUAL:${DEFAULT_OUTPUT_MODULE},tinyalsa>:tinyalsa.c>"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../src/libout123/modules/$<$<STREQUAL:${DEFAULT_OUTPUT_MODULE},pulse>:pulse.c>"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../src/libout123/modules/$<$<STREQUAL:${DEFAULT_OUTPUT_MODULE},jack>:jack.c>"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../src/libout123/modules/$<$<STREQUAL:${DEFAULT_OUTPUT_MODULE},win32>:win32.c>"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../src/libout123/modules/$<$<STREQUAL:${DEFAULT_OUTPUT_MODULE},win32_wasapi>:win32_wasapi.c>")
    target_link_libraries(defaultmodule PUBLIC
        $<$<STREQUAL:${DEFAULT_OUTPUT_MODULE},alsa>:ALSA::ALSA>
        $<$<STREQUAL:${DEFAULT_OUTPUT_MODULE},tinyalsa>:${TINYALSA_LIBRARIES}>
        $<$<STREQUAL:${DEFAULT_OUTPUT_MODULE},coreaudio>:${AUDIO_TOOLBOX}>
        $<$<STREQUAL:${DEFAULT_OUTPUT_MODULE},pulse>:${PULSE_LIBRARIES}>
        $<$<STREQUAL:${DEFAULT_OUTPUT_MODULE},jack>:${JACK_LIBRARIES}>
        $<$<STREQUAL:${DEFAULT_OUTPUT_MODULE},win32>:${WIN32_LIBRARIES}>
        $<$<STREQUAL:${DEFAULT_OUTPUT_MODULE},win32_wasapi>:${WIN32_WASAPI_LIBRARIES}>)
    if(DEFAULT_OUTPUT_MODULE STREQUAL "pulse")
        target_compile_options(defaultmodule PRIVATE ${PULSE_CFLAGS})
    elseif(DEFAULT_OUTPUT_MODULE STREQUAL "jack")
        target_compile_options(defaultmodule PRIVATE ${JACK_CFLAGS})
    elseif(DEFAULT_OUTPUT_MODULE STREQUAL "tinyalsa")
        target_compile_options(defaultmodule PRIVATE ${TINYALSA_CFLAGS})
    endif()
    if(BUILD_SHARED_LIBS)
        set_target_properties(defaultmodule PROPERTIES POSITION_INDEPENDENT_CODE ON)
    endif()
else()
    set(CMAKE_SHARED_MODULE_PREFIX "")
    list(FIND OUTPUT_MODULES coreaudio INDEX)
    if(NOT INDEX EQUAL -1)
        add_library(output_coreaudio MODULE "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../src/libout123/modules/coreaudio.c")
        target_link_libraries(output_coreaudio PRIVATE ${AUDIO_TOOLBOX})
        install(TARGETS output_coreaudio
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}"
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}"
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}/${PROJECT_NAME}")
    endif()
    list(FIND OUTPUT_MODULES alsa INDEX)
    if(NOT INDEX EQUAL -1)
        add_library(output_alsa MODULE "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../src/libout123/modules/alsa.c")
        target_link_libraries(output_alsa PRIVATE ALSA::ALSA)
        install(TARGETS output_alsa
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}"
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}"
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}/${PROJECT_NAME}")
    endif()
    list(FIND OUTPUT_MODULES pulse INDEX)
    if(NOT INDEX EQUAL -1)
        add_library(output_pulse MODULE "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../src/libout123/modules/pulse.c")
        target_compile_options(output_pulse PRIVATE ${PULSE_CFLAGS})
        target_link_libraries(output_pulse PRIVATE ${PULSE_LIBRARIES})
        install(TARGETS output_pulse
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}"
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}"
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}/${PROJECT_NAME}")
    endif()
    list(FIND OUTPUT_MODULES jack INDEX)
    if(NOT INDEX EQUAL -1)
        add_library(output_jack MODULE "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../src/libout123/modules/jack.c"
            $<TARGET_OBJECTS:compat_str>)
        target_compile_options(output_jack PRIVATE ${JACK_CFLAGS})
        target_link_libraries(output_jack PRIVATE ${JACK_LIBRARIES})
        install(TARGETS output_jack
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}"
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}"
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}/${PROJECT_NAME}")
    endif()
    list(FIND OUTPUT_MODULES tinyalsa INDEX)
    if(NOT INDEX EQUAL -1)
        add_library(output_tinyalsa MODULE "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../src/libout123/modules/tinyalsa.c")
        target_compile_options(output_tinyalsa PRIVATE ${TINYALSA_CFLAGS})
        target_link_libraries(output_tinyalsa PRIVATE ${TINYALSA_LIBRARIES})
        install(TARGETS output_tinyalsa
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}"
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}"
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}/${PROJECT_NAME}")
    endif()
    list(FIND OUTPUT_MODULES win32 INDEX)
    if(NOT INDEX EQUAL -1)
        add_library(output_win32 MODULE "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../src/libout123/modules/win32.c")
        target_link_libraries(output_win32 PRIVATE ${WIN32_LIBRARIES})
        install(TARGETS output_win32
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}"
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}"
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}/${PROJECT_NAME}")
    endif()
    list(FIND OUTPUT_MODULES win32_wasapi INDEX)
    if(NOT INDEX EQUAL -1)
        add_library(output_win32_wasapi MODULE "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../src/libout123/modules/win32_wasapi.c"
            $<TARGET_OBJECTS:compat_str>)
        target_link_libraries(output_win32_wasapi PRIVATE ${WIN32_WASAPI_LIBRARIES})
        install(TARGETS output_win32_wasapi
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}"
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}"
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}/${PROJECT_NAME}")
    endif()
endif()
