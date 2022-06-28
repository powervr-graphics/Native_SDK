cmake_minimum_required(VERSION 3.18.0 FATAL_ERROR)
project(glslang)

# Ensure that GLSLang only gets added once 
if(TARGET glslang)
	return()
endif()

# Set some variables in the cache that control the build for GLSLang
# We want to ensure that tests and external are turned off
set(BUILD_TESTING OFF CACHE INTERNAL "Build GLSLang Tests")
set(ENABLE_HLSL OFF CACHE INTERNAL "Build GLSLang HLSL compiler")
set(BUILD_EXTERNAL OFF CACHE INTERNAL "Build GLSLang external")

# If we are cross compiling, then we don't want to build GLSLangValidator for the target platform
# Because GLSLangValidator isn't used in the released binaries
# Instead we use the GLSLangValidator target on the host system to compile the Vulkan Shaders
# 
# However CMAKE_CROSS_COMPILING doesn't actually guarentee that we're cross compiling
# It just means that CMAKE_SYSTEM_NAME was set manually by a tool chain
if(CMAKE_CROSSCOMPILING)
	message(STATUS "Checking if system is actually cross compiling")
	if(NOT ("${CMAKE_SYSTEM_NAME}" STREQUAL "${CMAKE_HOST_SYSTEM_NAME}" 
		AND "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "${CMAKE_HOST_SYSTEM_PROCESSOR}"))

		message(STATUS "Cross-compilation detected, disabling GLSLangValidator builds")
		set(ENABLE_GLSLANG_BINARIES OFF CACHE INTERNAL "Build GLSLangValidator standalone executables")
	
	else()
		message(STATUS "Toolchain file passed, but target and host system are the same, enabling GLSLangValidator builds")
		set(ENABLE_GLSLANG_BINARIES ON CACHE INTERNAL "Build GLSLangValidator standalone executables")
	endif()
else()
	set(ENABLE_GLSLANG_BINARIES ON CACHE INTERNAL "Build GLSLangValidator standalone executables")
endif()

# Add the glslang source into the build tree
add_subdirectory(source "${CMAKE_CURRENT_BINARY_DIR}/build")

# If the GLSLangValidator Binaries were not built as part of the host targetted build
# Then we need to create an imported target which has a version of the binaries which 
# Can be executed on the host system. This is handled in the prebuilt subdirectory
if(NOT ${ENABLE_GLSLANG_BINARIES})
	add_subdirectory(prebuilt)
endif()