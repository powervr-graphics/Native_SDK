cmake_minimum_required(VERSION 3.10)

# EVERYTHING IN THIS FILE IS BASICALLY OPTIONAL. It is our recommended compiler configuration and some tools. This is applied to all targets via the "enable_sdk_options_for_target"

include(CheckCXXCompilerFlag)

# Options that can be set
option(PVR_ENABLE_FAST_MATH "If enabled, attempt to enable fast-math." ON)

if(WIN32)
	if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
		option(PVR_MSVC_ENABLE_LTCG "If enabled, use Link Time Code Generation for non-debug builds." ON)
		option(PVR_MSVC_ENABLE_JUST_MY_CODE "If enabled, enable 'Just My Code' feature." ON)
		option(PVR_MSVC_USE_STATIC_RUNTIME "If enabled, build against the static, rather than the dynamic, runtime." OFF)
	endif()
elseif(UNIX)
	option(PVR_UNIX_USE_GOLD_LINKER "If enabled, use the Gold linker instead of ld if available." ON)
endif()

function(enable_sdk_options_for_target THETARGET)
	if (NOT TARGET ${THETARGET})
		return()
	endif()
	if (WIN32)
		if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
			if(PVR_MSVC_ENABLE_LTCG)
				# CMake 3.13+ method preferred, but fallback to property for older versions if needed.
				# Since we require 3.10, we still use properties or flags manually if target_link_options isn't available.
                # However, many Android/NDK setups now support recent CMake.
                # If we assume 3.13+, we could use target_link_options. 
                # For now, sticking to robust property appending.
				set_property(TARGET ${THETARGET} APPEND PROPERTY LINK_FLAGS_RELEASE "/LTCG:INCREMENTAL")
				set_property(TARGET ${THETARGET} APPEND PROPERTY LINK_FLAGS_MINSIZEREL "/LTCG:INCREMENTAL")
				set_property(TARGET ${THETARGET} APPEND PROPERTY LINK_FLAGS_RELWITHDEBINFO "/LTCG:INCREMENTAL")
			endif()

			if(PVR_MSVC_ENABLE_JUST_MY_CODE)
				# Enable "Just My Code" feature introduced in MSVC 15.8 (Visual Studio 2017)
				if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "19.15" )
					target_compile_options(${THETARGET} PRIVATE "$<$<CONFIG:DEBUG>:/JMC>")
				endif()
			endif()

			# Add the all-important fast-math flag
			if(PVR_ENABLE_FAST_MATH)
				CHECK_CXX_COMPILER_FLAG(/fp:fast COMPILER_SUPPORTS_FAST_MATH)
				if(COMPILER_SUPPORTS_FAST_MATH)
					target_compile_options(${THETARGET} PRIVATE "/fp:fast")
				endif()
			endif()
		endif()
	elseif(UNIX)
		if(NOT APPLE AND PVR_UNIX_USE_GOLD_LINKER)
			# Use Gold Linker by default when it is supported
			execute_process(COMMAND ${CMAKE_C_COMPILER} -fuse-ld=gold -Wl,--version ERROR_QUIET OUTPUT_VARIABLE ld_version)
			if("${ld_version}" MATCHES "GNU gold")
				set_property(TARGET ${THETARGET} APPEND_STRING PROPERTY LINK_FLAGS " -fuse-ld=gold -Wl,--disable-new-dtags")
			endif()
		endif()

		if(PVR_ENABLE_FAST_MATH)
			if((CMAKE_CXX_COMPILER_ID MATCHES "Clang") OR (CMAKE_CXX_COMPILER_ID MATCHES "GNU"))
				CHECK_CXX_COMPILER_FLAG(-ffast-math COMPILER_SUPPORTS_FAST_MATH)
				if(COMPILER_SUPPORTS_FAST_MATH)
					target_compile_options(${THETARGET} PRIVATE "-ffast-math")
				endif()
			endif()
		endif()
	endif()
endfunction()
