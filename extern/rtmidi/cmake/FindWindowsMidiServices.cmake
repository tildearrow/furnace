# Finds the in-box Windows MIDI Services API, Windows.Devices.Midi2.
#
# The API is part of Windows 11 25H2 and later. To compile against it, RtMidi
# needs its C++/WinRT projection, which comes from one of two places:
#
#   1. A Windows SDK whose C++/WinRT headers already include
#      winrt/Windows.Devices.Midi2.h. Nothing is generated.
#   2. Windows.Devices.Midi2.winmd, from which the projection is generated with
#      cppwinrt.exe. Until a Windows SDK includes the API, the metadata comes
#      from Microsoft's Windows.Devices.Midi2 NuGet package (ref/native/).
#      Point at it with WindowsMidiServices_WINMD (the file) or
#      WindowsMidiServices_ROOT (a directory containing it, or the unpacked
#      package), as a CMake or environment variable.
#
# Run CppWinRT's find module first; this one uses CppWinRT_INCLUDE_DIR.
#
# Results:
#   WindowsMidiServices_FOUND
#   WindowsMidiServices_SDK_PROJECTION  TRUE when the SDK already provides the
#                                       projection and nothing is generated
#   WindowsMidiServices_WINMD           the metadata to generate from, when not

include(FindPackageHandleStandardArgs)

if(NOT WIN32)
  set(WindowsMidiServices_FOUND FALSE)
  set(WindowsMidiServices_NOT_FOUND_MESSAGE "Windows MIDI Services is only available on Windows.")
  return()
endif()

set(WindowsMidiServices_SDK_PROJECTION FALSE)

if(NOT WindowsMidiServices_WINMD AND DEFINED ENV{WindowsMidiServices_WINMD})
  file(TO_CMAKE_PATH "$ENV{WindowsMidiServices_WINMD}" _wms_env_winmd)
  set(WindowsMidiServices_WINMD "${_wms_env_winmd}" CACHE FILEPATH
      "Windows.Devices.Midi2.winmd to generate the C++/WinRT projection from")
endif()

if(NOT WindowsMidiServices_WINMD)
  set(_wms_roots)
  if(DEFINED WindowsMidiServices_ROOT)
    list(APPEND _wms_roots "${WindowsMidiServices_ROOT}")
  endif()
  if(DEFINED ENV{WindowsMidiServices_ROOT})
    file(TO_CMAKE_PATH "$ENV{WindowsMidiServices_ROOT}" _wms_env_root)
    list(APPEND _wms_roots "${_wms_env_root}")
  endif()

  if(_wms_roots)
    find_file(WindowsMidiServices_WINMD
      NAMES Windows.Devices.Midi2.winmd
      PATHS ${_wms_roots}
      PATH_SUFFIXES "" "ref/native"
      NO_DEFAULT_PATH)
  endif()
endif()

# Without explicit metadata, use a Windows SDK that already carries the API.
if(NOT WindowsMidiServices_WINMD AND CppWinRT_INCLUDE_DIR
   AND EXISTS "${CppWinRT_INCLUDE_DIR}/winrt/Windows.Devices.Midi2.h")
  set(WindowsMidiServices_SDK_PROJECTION TRUE)
endif()

if(WindowsMidiServices_SDK_PROJECTION)
  find_package_handle_standard_args(WindowsMidiServices
    REQUIRED_VARS CppWinRT_INCLUDE_DIR)
else()
  find_package_handle_standard_args(WindowsMidiServices
    REQUIRED_VARS WindowsMidiServices_WINMD
    FAIL_MESSAGE "Windows.Devices.Midi2 metadata not found. This Windows SDK does not include the API. Set WindowsMidiServices_WINMD to Windows.Devices.Midi2.winmd (for example from the Windows.Devices.Midi2 NuGet package's ref/native directory), or WindowsMidiServices_ROOT to the directory containing it.")
endif()

mark_as_advanced(WindowsMidiServices_WINMD)
