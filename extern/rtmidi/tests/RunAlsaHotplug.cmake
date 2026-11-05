execute_process(COMMAND "${TEST_EXECUTABLE}"
  RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE errors TIMEOUT 30)
if("${result}" STREQUAL "77")
  message(STATUS "RtMidi hotplug test skipped: ${output}")
elseif(NOT "${result}" STREQUAL "0")
  message(FATAL_ERROR "RtMidi hotplug test failed (${result}):\n${output}\n${errors}")
else()
  message(STATUS "RtMidi hotplug test passed")
endif()
