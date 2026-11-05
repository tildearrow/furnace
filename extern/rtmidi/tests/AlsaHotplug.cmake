# Build the ALSA-only fixture separately so linker wrappers cannot affect users
# of the library. The source uses the same platform/version guards as RtMidi.
function(rtmidi_add_hotplug_test library source_dir)
  find_package(ALSA REQUIRED)
  find_package(Threads REQUIRED)
  add_executable(rtmidi_alsahotplug
    "${source_dir}/tests/alsahotplug.cpp" "${source_dir}/RtMidi.cpp")
  target_compile_definitions(rtmidi_alsahotplug PRIVATE RTMIDI_API_ALSA)
  target_compile_options(rtmidi_alsahotplug PRIVATE -URTMIDI_API_JACK -U__UNIX_JACK__)
  target_link_libraries(rtmidi_alsahotplug PRIVATE ALSA::ALSA Threads::Threads)
  target_link_options(rtmidi_alsahotplug PRIVATE
    -Wl,--wrap=snd_seq_client_info_get_card -Wl,--wrap=realpath -Wl,--wrap=__realpath_chk
    -Wl,--wrap=pipe2 -Wl,--wrap=pthread_create -Wl,--wrap=snd_seq_open -Wl,--wrap=snd_seq_event_input
    -Wl,--wrap=snd_seq_subscribe_port -Wl,--wrap=snd_seq_poll_descriptors
    -Wl,--wrap=snd_seq_query_next_client)
  add_dependencies(${library} rtmidi_alsahotplug)

  # Cross-built binaries cannot run on the build host without an emulator.
  if(CMAKE_CROSSCOMPILING)
    message(STATUS "RtMidi hotplug test built but not run while cross-compiling")
    return()
  endif()
  enable_testing()
  add_test(NAME rtmidi_alsahotplug COMMAND rtmidi_alsahotplug)
  set_tests_properties(rtmidi_alsahotplug PROPERTIES SKIP_RETURN_CODE 77 TIMEOUT 30)
  add_custom_target(rtmidi_hotplug_check ALL
    COMMAND "${CMAKE_COMMAND}" "-DTEST_EXECUTABLE=$<TARGET_FILE:rtmidi_alsahotplug>"
      -P "${source_dir}/tests/RunAlsaHotplug.cmake"
    DEPENDS rtmidi_alsahotplug VERBATIM)
  add_dependencies(${library} rtmidi_hotplug_check)
endfunction()
