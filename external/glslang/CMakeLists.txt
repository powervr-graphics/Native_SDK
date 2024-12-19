cmake_minimum_required(VERSION 3.10)

# Ensure that GLSLang only gets added once 
if(TARGET glslang)
	return()
endif()

# Each file needs a different URL and HASH
if(CMAKE_HOST_WIN32)
	set(HOST_EXECUTABLE_SUFFIX ".exe" CACHE INTERNAL "")
elseif(CMAKE_HOST_UNIX)
	set(HOST_EXECUTABLE_SUFFIX "" CACHE INTERNAL "")
endif()

if(CMAKE_CROSSCOMPILING AND NOT ("${CMAKE_SYSTEM_NAME}" STREQUAL "${CMAKE_HOST_SYSTEM_NAME}" AND "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "${CMAKE_HOST_SYSTEM_PROCESSOR}"))
	message(STATUS "Cross compilation detected, using prebuilt glslangValidator")
	set(IS_CROSS_COMPILATION ON CACHE INTERNAL "")
else()
	message(STATUS "Cross compilation not detected, building glslangValidator")
	set(IS_CROSS_COMPILATION OFF CACHE INTERNAL "")
endif()

if(PVR_GLSLANG_VALIDATOR_INSTALL_DIR)
	set(glslangValidator_BINARY_LOCATION "${PVR_GLSLANG_VALIDATOR_INSTALL_DIR}/bin/glslangValidator${HOST_EXECUTABLE_SUFFIX}")
else()
	# For the case of cross compiling from Windows or Linux the glslangValidator executable used is a prebuilt one
	if(IS_CROSS_COMPILATION)
		if(CMAKE_HOST_WIN32)
			set(glslangValidator_BINARY_LOCATION "${CMAKE_CURRENT_LIST_DIR}/prebuilt/glslangValidator-win-64${HOST_EXECUTABLE_SUFFIX}")
		elseif(CMAKE_HOST_UNIX)
			set(glslangValidator_BINARY_LOCATION "${CMAKE_CURRENT_LIST_DIR}/prebuilt/glslangValidator-linux-x86_64${HOST_EXECUTABLE_SUFFIX}")
		endif()
	else()
		set(glslangValidator_BINARY_LOCATION "${CMAKE_CURRENT_LIST_DIR}/StandAlone/${CMAKE_BUILD_TYPE}/glslangValidator${HOST_EXECUTABLE_SUFFIX}")
	endif()
endif()

if(NOT PVR_PREBUILT_DEPENDENCIES)
	set(glslang_SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/source CACHE INTERNAL "")

	# Set some variables in the cache that control the build for GLSLang
	# We want to ensure that tests and external are turned off
	set(BUILD_TESTING OFF CACHE INTERNAL "Build GLSLang Tests")
	set(ENABLE_HLSL OFF CACHE INTERNAL "Build GLSLang HLSL compiler")
	set(BUILD_EXTERNAL OFF CACHE INTERNAL "Build GLSLang external")

	if (IS_CROSS_COMPILATION)	
		# Set the ENABLE_GLSLANG_BINARIES flag for the glslang library so glslangvalidator is not built, as for Android precompiled binaries are used
		set(ENABLE_GLSLANG_BINARIES OFF CACHE INTERNAL "")
	else()
		# Set the ENABLE_GLSLANG_BINARIES flag for the glslang library to build glslangvalidator
		set(ENABLE_GLSLANG_BINARIES ON CACHE INTERNAL "")
	endif()

	add_subdirectory("${glslang_SRC_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/build")

	if (NOT IS_CROSS_COMPILATION)	
		export(TARGETS glslangValidator FILE "${CMAKE_CURRENT_BINARY_DIR}/build/glslangValidatorConfig.cmake")
		file(COPY glslangValidatorConfig.cmake.in DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/" NO_SOURCE_PERMISSIONS)
		configure_file("${CMAKE_CURRENT_BINARY_DIR}/glslangValidatorConfig.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/glslangValidatorConfig.cmake" COPYONLY)
	endif()

	export(TARGETS OGLCompiler FILE "${CMAKE_CURRENT_BINARY_DIR}/build/OGLCompilerConfig.cmake")
	export(TARGETS OSDependent FILE "${CMAKE_CURRENT_BINARY_DIR}/build/OSDependentConfig.cmake")
	export(TARGETS MachineIndependent FILE "${CMAKE_CURRENT_BINARY_DIR}/build/MachineIndependentConfig.cmake")
	export(TARGETS GenericCodeGen FILE "${CMAKE_CURRENT_BINARY_DIR}/build/GenericCodeGenConfig.cmake")
	export(TARGETS glslang FILE "${CMAKE_CURRENT_BINARY_DIR}/build/glslangConfig.cmake")
	export(TARGETS SPIRV FILE "${CMAKE_CURRENT_BINARY_DIR}/build/SPIRVConfig.cmake")

	file(COPY OGLCompilerConfig.cmake.in DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/" NO_SOURCE_PERMISSIONS)
	file(COPY OSDependentConfig.cmake.in DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/" NO_SOURCE_PERMISSIONS)
	file(COPY MachineIndependentConfig.cmake.in DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/" NO_SOURCE_PERMISSIONS)
	file(COPY GenericCodeGenConfig.cmake.in DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/" NO_SOURCE_PERMISSIONS)
	file(COPY glslangConfig.cmake.in DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/" NO_SOURCE_PERMISSIONS)
	file(COPY SPIRVConfig.cmake.in DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/" NO_SOURCE_PERMISSIONS)

	configure_file("${CMAKE_CURRENT_BINARY_DIR}/OGLCompilerConfig.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/OGLCompilerConfig.cmake" COPYONLY)
	configure_file("${CMAKE_CURRENT_BINARY_DIR}/OSDependentConfig.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/OSDependentConfig.cmake" COPYONLY)
	configure_file("${CMAKE_CURRENT_BINARY_DIR}/MachineIndependentConfig.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/MachineIndependentConfig.cmake" COPYONLY)
	configure_file("${CMAKE_CURRENT_BINARY_DIR}/GenericCodeGenConfig.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/GenericCodeGenConfig.cmake" COPYONLY)
	configure_file("${CMAKE_CURRENT_BINARY_DIR}/glslangConfig.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/glslangConfig.cmake" COPYONLY)
	configure_file("${CMAKE_CURRENT_BINARY_DIR}/SPIRVConfig.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/SPIRVConfig.cmake" COPYONLY)

	set_property(GLOBAL APPEND PROPERTY PVR_EXTERNAL_TARGETS glslang)
endif()

if(IS_CROSS_COMPILATION OR PVR_GLSLANG_VALIDATOR_INSTALL_DIR)
	add_executable(glslangValidator IMPORTED GLOBAL)
	set_target_properties(glslangValidator PROPERTIES IMPORTED_LOCATION "${glslangValidator_BINARY_LOCATION}")
endif()
