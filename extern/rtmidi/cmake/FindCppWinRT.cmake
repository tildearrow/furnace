include(FindPackageHandleStandardArgs)

if(NOT WIN32)
  set(CppWinRT_FOUND FALSE)
  set(CppWinRT_NOT_FOUND_MESSAGE "CppWinRT is only available on Windows.")
  return()
endif()

set(_cppwinrt_sdk_root "C:/Program Files (x86)/Windows Kits/10")
if(DEFINED ENV{WindowsSdkDir} AND EXISTS "$ENV{WindowsSdkDir}")
  file(TO_CMAKE_PATH "$ENV{WindowsSdkDir}" _cppwinrt_sdk_root)
endif()

set(_cppwinrt_sdk_versions)
if(CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION)
  list(APPEND _cppwinrt_sdk_versions "${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}")
endif()
if(DEFINED ENV{WindowsSDKVersion})
  file(TO_CMAKE_PATH "$ENV{WindowsSDKVersion}" _cppwinrt_env_sdk_version)
  string(REGEX REPLACE "/$" "" _cppwinrt_env_sdk_version "${_cppwinrt_env_sdk_version}")
  if(_cppwinrt_env_sdk_version)
    list(APPEND _cppwinrt_sdk_versions "${_cppwinrt_env_sdk_version}")
  endif()
endif()
if(EXISTS "${_cppwinrt_sdk_root}/Include")
  file(GLOB _cppwinrt_include_candidates LIST_DIRECTORIES true "${_cppwinrt_sdk_root}/Include/*")
  foreach(_cppwinrt_candidate IN LISTS _cppwinrt_include_candidates)
    if(EXISTS "${_cppwinrt_candidate}/cppwinrt/winrt/base.h")
      get_filename_component(_cppwinrt_candidate_name "${_cppwinrt_candidate}" NAME)
      list(APPEND _cppwinrt_sdk_versions "${_cppwinrt_candidate_name}")
    endif()
  endforeach()
endif()
list(REMOVE_DUPLICATES _cppwinrt_sdk_versions)
list(SORT _cppwinrt_sdk_versions COMPARE NATURAL ORDER DESCENDING)

foreach(_cppwinrt_version IN LISTS _cppwinrt_sdk_versions)
  if(NOT CppWinRT_INCLUDE_DIR AND EXISTS "${_cppwinrt_sdk_root}/Include/${_cppwinrt_version}/cppwinrt/winrt/base.h")
    set(CppWinRT_INCLUDE_DIR "${_cppwinrt_sdk_root}/Include/${_cppwinrt_version}/cppwinrt")
    set(CppWinRT_SDK_VERSION "${_cppwinrt_version}")
  endif()
  if(NOT CppWinRT_WINDOWS_WINMD AND EXISTS "${_cppwinrt_sdk_root}/UnionMetadata/${_cppwinrt_version}/Windows.winmd")
    set(CppWinRT_WINDOWS_WINMD "${_cppwinrt_sdk_root}/UnionMetadata/${_cppwinrt_version}/Windows.winmd")
    if(NOT CppWinRT_SDK_VERSION)
      set(CppWinRT_SDK_VERSION "${_cppwinrt_version}")
    endif()
  endif()
endforeach()

set(_cppwinrt_bin_hints)
foreach(_cppwinrt_version IN LISTS _cppwinrt_sdk_versions)
  list(APPEND _cppwinrt_bin_hints
    "${_cppwinrt_sdk_root}/bin/${_cppwinrt_version}/x64"
    "${_cppwinrt_sdk_root}/bin/${_cppwinrt_version}/x86"
    "${_cppwinrt_sdk_root}/bin/${_cppwinrt_version}/arm64")
endforeach()

find_program(CppWinRT_EXECUTABLE
  NAMES cppwinrt
  HINTS ${_cppwinrt_bin_hints})

find_package_handle_standard_args(CppWinRT
  REQUIRED_VARS CppWinRT_EXECUTABLE CppWinRT_INCLUDE_DIR CppWinRT_WINDOWS_WINMD)

mark_as_advanced(CppWinRT_EXECUTABLE CppWinRT_INCLUDE_DIR CppWinRT_WINDOWS_WINMD)
