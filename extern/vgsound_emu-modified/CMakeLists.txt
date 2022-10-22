#
# License: Zlib
# see https://gitlab.com/cam900/vgsound_emu/-/blob/main/LICENSE for more details
#
# Copyright holder(s): cam900
# CMake for vgsound_emu
#

cmake_minimum_required(VERSION 3.0)
project(vgsound_emu
	VERSION 2.1.1
	LANGUAGES CXX)

enable_language(CXX)

option(VGSOUND_EMU_ES5504 "Use ES5504 core" ON)
option(VGSOUND_EMU_ES5505 "Use ES5505 core" ON)
option(VGSOUND_EMU_ES5506 "Use ES5506 core" ON)
option(VGSOUND_EMU_K005289 "Use K005289 core" ON)
option(VGSOUND_EMU_K007232 "Use K007232 core" ON)
option(VGSOUND_EMU_K053260 "Use K053260 core" ON)
option(VGSOUND_EMU_MSM6295 "Use MSM6295 core" ON)
option(VGSOUND_EMU_NAMCO_163 "Use Namco 163 core" ON)
option(VGSOUND_EMU_SCC "Use SCC core" ON)
option(VGSOUND_EMU_VRCVI "Use VRC VI core" ON)
option(VGSOUND_EMU_X1_010 "Use X1-010 core" ON)

message(STATUS "Host: ${CMAKE_HOST_SYSTEM_NAME}, ${CMAKE_HOST_SYSTEM_PROCESSOR}")
message(STATUS "Target: ${CMAKE_SYSTEM_NAME}, ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "CMake version: ${CMAKE_VERSION}")
message(STATUS "Generator: ${CMAKE_GENERATOR}")
message(STATUS "Extra generator: ${CMAKE_EXTRA_GENERATOR}")
message(STATUS "Make program: ${CMAKE_MAKE_PROGRAM}")

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.1.0")
	message(STATUS "Generating C++11 compile features:")

	if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.8.0")
		set(cxx_features cxx_std_11)
	else()
		set(cxx_features
			cxx_template_template_parameters
			cxx_alias_templates
			cxx_alignas
			cxx_alignof
			cxx_attributes
			cxx_auto_type
			cxx_constexpr
			cxx_decltype
			cxx_decltype_incomplete_return_types
			cxx_default_function_template_args
			cxx_defaulted_functions
			cxx_defaulted_move_initializers
			cxx_delegating_constructors
			cxx_deleted_functions
			cxx_enum_forward_declarations
			cxx_explicit_conversions
			cxx_extended_friend_declarations
			cxx_extern_templates
			cxx_final
			cxx_func_identifier
			cxx_generalized_initializers
			cxx_inheriting_constructors
			cxx_inline_namespaces
			cxx_lambdas
			cxx_local_type_template_args
			cxx_long_long_type
			cxx_noexcept
			cxx_nonstatic_member_init
			cxx_nullptr
			cxx_override
			cxx_range_for
			cxx_raw_string_literals
			cxx_reference_qualified_functions
			cxx_right_angle_brackets
			cxx_rvalue_references
			cxx_sizeof_member
			cxx_static_assert
			cxx_strong_enums
			cxx_thread_local
			cxx_trailing_return_types
			cxx_unicode_literals
			cxx_uniform_initialization
			cxx_unrestricted_unions
			cxx_user_literals
			cxx_variadic_macros
			cxx_variadic_templates)
	endif()

	message(STATUS "${cxx_features}")
endif()

set(CORE_SOURCE "")
set(EMU_SOURCE "")

# Core functions
list(APPEND CORE_SOURCE
	vgsound_emu/src/core/core.hpp
	vgsound_emu/src/core/util.hpp
	vgsound_emu/src/core/util/clock_pulse.hpp
	vgsound_emu/src/core/util/mem_intf.hpp
)

# Dialogic ADPCM
if(VGSOUND_EMU_MSM6295)
	list(APPEND CORE_SOURCE
		vgsound_emu/src/core/vox/vox.hpp
		vgsound_emu/src/core/vox/vox.cpp
	)
	message(STATUS "Using Dialogic ADPCM core")
endif()

# ES5504, ES5505, ES5506
if(VGSOUND_EMU_ES5504 OR VGSOUND_EMU_ES5505 OR VGSOUND_EMU_ES5506)
	list(APPEND EMU_SOURCE
		vgsound_emu/src/es550x/es550x.hpp

		vgsound_emu/src/es550x/es550x.cpp
		vgsound_emu/src/es550x/es550x_alu.cpp
		vgsound_emu/src/es550x/es550x_filter.cpp
	)

	if(VGSOUND_EMU_ES5504)
		list(APPEND EMU_SOURCE
			vgsound_emu/src/es550x/es5504.hpp
			vgsound_emu/src/es550x/es5504.cpp
		)
		message(STATUS "Using ES5504 core")
	endif()

	if(VGSOUND_EMU_ES5505)
		list(APPEND EMU_SOURCE
			vgsound_emu/src/es550x/es5505.hpp
			vgsound_emu/src/es550x/es5505.cpp
		)
		message(STATUS "Using ES5505 core")
	endif()

	if(VGSOUND_EMU_ES5506)
		list(APPEND EMU_SOURCE
			vgsound_emu/src/es550x/es5506.hpp
			vgsound_emu/src/es550x/es5506.cpp
		)
		message(STATUS "Using ES5506 core")
	endif()
endif()

# K005289
if(VGSOUND_EMU_K005289)
	list(APPEND EMU_SOURCE
		vgsound_emu/src/k005289/k005289.hpp
		vgsound_emu/src/k005289/k005289.cpp
	)
	message(STATUS "Using K005289 core")
endif()

# K007232
if(VGSOUND_EMU_K007232)
	list(APPEND EMU_SOURCE
		vgsound_emu/src/k007232/k007232.hpp
		vgsound_emu/src/k007232/k007232.cpp
	)
	message(STATUS "Using K007232 core")
endif()

# K053260
if(VGSOUND_EMU_K053260)
	list(APPEND EMU_SOURCE
		vgsound_emu/src/k053260/k053260.hpp
		vgsound_emu/src/k053260/k053260.cpp
	)
	message(STATUS "Using K053260 core")
endif()

# MSM6295
if(VGSOUND_EMU_MSM6295)
	list(APPEND EMU_SOURCE
		vgsound_emu/src/msm6295/msm6295.hpp
		vgsound_emu/src/msm6295/msm6295.cpp
	)
	message(STATUS "Using MSM6295 core")
endif()

# Namco 163
if(VGSOUND_EMU_NAMCO_163)
	list(APPEND EMU_SOURCE
		vgsound_emu/src/n163/n163.hpp
		vgsound_emu/src/n163/n163.cpp
	)
	message(STATUS "Using Namco 163 core")
endif()

# SCC
if(VGSOUND_EMU_SCC)
	list(APPEND EMU_SOURCE
		vgsound_emu/src/scc/scc.hpp
		vgsound_emu/src/scc/scc.cpp
	)
	message(STATUS "Using SCC core")
endif()

# VRC VI
if(VGSOUND_EMU_VRCVI)
	list(APPEND EMU_SOURCE
		vgsound_emu/src/vrcvi/vrcvi.hpp
		vgsound_emu/src/vrcvi/vrcvi.cpp
	)
	message(STATUS "Using VRC VI core")
endif()

# X1-010
if(VGSOUND_EMU_X1_010)
	list(APPEND EMU_SOURCE
		vgsound_emu/src/x1_010/x1_010.hpp
		vgsound_emu/src/x1_010/x1_010.cpp
	)
	message(STATUS "Using X1-010 core")
endif()

add_library(vgsound_emu STATIC ${CORE_SOURCE} ${EMU_SOURCE})

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.1.0")
	set_target_properties(vgsound_emu
		PROPERTIES
		CXX_STANDARD 11
		CXX_STANDARD_REQUIRED ON
		CXX_EXTENSIONS OFF)
	target_compile_features(vgsound_emu PRIVATE ${cxx_features})
	message(STATUS "C++11 Enabled")
endif()

target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# for Test purpose (GCC)
# target_compile_options(vgsound_emu PRIVATE -Wall -Wextra -Werror)