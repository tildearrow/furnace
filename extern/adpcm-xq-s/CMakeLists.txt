cmake_minimum_required(VERSION 3.16)

# set the project name
project(adpcm-xq)

# lots of warnings and all warnings as errors
# add_compile_options(-Wall -Wextra )

# define as library
add_library (adpcm-xq adpcm-xq.c)

# build executable
add_executable (adpcm-xq-exe adpcm-lib.c adpcm-xq.c)
set_property(TARGET adpcm-xq-exe PROPERTY OUTPUT_NAME adpcm-xq)

# define location for header files
target_include_directories(adpcm-xq PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )

