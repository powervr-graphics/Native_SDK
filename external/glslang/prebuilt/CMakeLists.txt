cmake_minimum_required(VERSION 3.18.0 FATAL_ERROR)
project(GlslangValidatorPrebuild)

# This CMake file handles the importing of a glslangValidator target
# If there isn't an executable for the host availble, then we can 
# Point the user towards building GlslangValidator and adding it here

# Ensure that only one glslang validator binary is used 
if(TARGET glslangValidator)
	return()
endif()

# Choose the right host executable for glslangValidator 
if("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Windows" OR "${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Cygwin")
	set(HOST_VALIDATOR_BIN "${CMAKE_CURRENT_LIST_DIR}/glslangValidator-win-64.exe")

elseif("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Linux")
	if("${CMAKE_HOST_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
		set(HOST_VALIDATOR_BIN "${CMAKE_CURRENT_LIST_DIR}/glslangValidator-linux-x86_64")
	else()
		message(FATAL_ERROR "We have not supplied a glslangValidator binary for Linux on ${CMAKE_HOST_SYSTEM_PROCESSOR} systems")
	endif()

elseif("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Darwin")
	# We don't support targetting Apple, but maybe you're cross compiling
	# for android on macOS. Might not work very well though
	set(HOST_VALIDATOR_BIN "${CMAKE_CURRENT_LIST_DIR}/glslangValidator-osx")

endif()

# If we stil haven't found the host executable for glslangValidator
# in our list of supplied precompiled binaries, then try the 
# Vulkan SDK, make sure to warn the user first
if(NOT DEFINED HOST_VALIDATOR_BIN)
	message(WARNING "Could not find ${CMAKE_HOST_SYSTEM_NAME}:${CMAKE_HOST_SYSTEM_PROCESSOR}" 
		"compatable prebuilt glslangValidator for compiling SPIRV while crosscompiling."
		"Attempting to use version in Vukan SDK environment variable")
	set(HOST_VALIDATOR_BIN "$ENV{VULKAN_SDK}/Bin/glslangValidator")
endif()

# Now that we are guarenteed to have a value for HOST_VALIDATOR_BIN
# Check that a file actually exists at that location
# If a file does exist, then import it as a target, or else throw
# A CMake error
if(NOT EXISTS ${HOST_VALIDATOR_BIN})
	message(FATAL_ERROR "Could not find a valid host executable for glslangValidator, tried \"${HOST_VALIDATOR_BIN}\"")
endif()
message(STATUS "Adding imported glslangValidator target")
add_executable(glslangValidator IMPORTED GLOBAL)
set_property(TARGET glslangValidator PROPERTY IMPORTED_LOCATION "${HOST_VALIDATOR_BIN}")