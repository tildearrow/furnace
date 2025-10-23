function(read_api_version project_version)

    file( READ "${CMAKE_CURRENT_SOURCE_DIR}/../../src/version.h" version_h )

    string( REGEX MATCH "#define +MPG123_MAJOR +([0-9]+)" result ${version_h} )
    set( major_version ${CMAKE_MATCH_1})
    string( REGEX MATCH "#define +MPG123_MINOR +([0-9]+)" result ${version_h} )
    set( minor_version ${CMAKE_MATCH_1})

    string( REGEX MATCH "#define +MPG123_PATCH +([0-9]+)" result ${version_h} )
    set( patch_version ${CMAKE_MATCH_1})

#    string( REGEX MATCH "#define +MPG123_SUFFIX +\"([^\"]+)\"" result ${version_h} )
#    set( version_suffix ${CMAKE_MATCH_1})
# CMake project() chokes on version with suffix, so give it just the numbers.
    set( ${project_version} ${major_version}.${minor_version}.${patch_version} PARENT_SCOPE)

endfunction()
