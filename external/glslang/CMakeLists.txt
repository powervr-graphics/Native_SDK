cmake_minimum_required(VERSION 3.3)

# Set glslang download directory - we use a downloads directory to allow us to maintain cleanliness
set(glslang_DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}/downloads/glslang_downloads CACHE INTERNAL "")

if(CMAKE_HOST_WIN32)
	set(glslangValidator_PACKAGE_NAME "glslang-master-windows-x64-Release")
	set(HOST_EXECUTABLE_SUFFIX ".exe" CACHE INTERNAL "")
elseif(CMAKE_HOST_APPLE)
	set(glslangValidator_PACKAGE_NAME "glslang-master-osx-Release")
	set(HOST_EXECUTABLE_SUFFIX "" CACHE INTERNAL "")
elseif(CMAKE_HOST_UNIX)
	set(glslangValidator_PACKAGE_NAME "glslang-master-linux-Release")
	set(HOST_EXECUTABLE_SUFFIX "" CACHE INTERNAL "")
endif()

if(PVR_GLSLANG_VALIDATOR_INSTALL_DIR)
	set(glslangValidator_BINARY_LOCATION "${PVR_GLSLANG_VALIDATOR_INSTALL_DIR}/bin/glslangValidator${HOST_EXECUTABLE_SUFFIX}")
else()
	set(glslangValidator_BINARY_LOCATION "${glslang_DOWNLOAD_DIR}/${glslangValidator_PACKAGE_NAME}/bin/glslangValidator${HOST_EXECUTABLE_SUFFIX}")
endif()

if(NOT PVR_PREBUILT_DEPENDENCIES)
	include(../../cmake/Functions.cmake)

	# Set current glslang version
	set(external_glslang_VERSION "7.11.3113" CACHE STRING "The exact version of GLSLANG to download from github")

	# Set the glslang source directory
	set(glslang_SRC_DIR ${CMAKE_CURRENT_BINARY_DIR}/src CACHE INTERNAL "")

	# Don't download if it is already present - this handles cases where internet connectivity may be limited but all packages are already available
	if(EXISTS ${glslang_DOWNLOAD_DIR}/${external_glslang_VERSION}.tar.gz AND EXISTS ${glslang_SRC_DIR}/CMakeLists.txt)
		set(glslang_URL "" CACHE INTERNAL "")
	else()
		set(glslang_URL "https://github.com/KhronosGroup/glslang/archive/${external_glslang_VERSION}.tar.gz" CACHE INTERNAL "")
		message("glslang was not found so will be downloaded: ${glslang_URL}")
	endif()

	# glslang
	download_external_project("glslang" "${CMAKE_CURRENT_BINARY_DIR}/download" "${glslang_SRC_DIR}" "${glslang_DOWNLOAD_DIR}" "${glslang_URL}" "${glslang_SRC_DIR}/CMakeLists.txt")

	# These options are used for both glslang and glslangValidator
	set(BUILD_TESTING OFF CACHE INTERNAL "")
	set(ENABLE_HLSL OFF CACHE INTERNAL "")

	# If CMAKE_CROSSCOMPILING then do not build glslangValidator using normal add_subdirectory logic
	if(CMAKE_CROSSCOMPILING OR APPLE OR PVR_GLSLANG_VALIDATOR_INSTALL_DIR)
		# See glslangValidator/CMakeLists.txt for how glslangValidator is built when cross-compiling
		set(ENABLE_GLSLANG_BINARIES OFF CACHE INTERNAL "")
	else()
		set(ENABLE_GLSLANG_BINARIES ON CACHE INTERNAL "")
	endif()

	add_subdirectory("${glslang_SRC_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/build")
	
	# If CMAKE_CROSSCOMPILING then we configure glslangValidator separately and import the executable
	if((CMAKE_CROSSCOMPILING OR APPLE) AND NOT PVR_GLSLANG_VALIDATOR_INSTALL_DIR)
		set(glslangValidator_base_URL "https://github.com/KhronosGroup/glslang/releases/download/${external_glslang_VERSION}")
		
		# Don't download if it is already present - this handles cases where internet connectivity may be limited but all packages are already available
		if(EXISTS ${glslang_DOWNLOAD_DIR}/${glslangValidator_PACKAGE_NAME}.zip AND EXISTS ${glslangValidator_BINARY_LOCATION})
			set(glslangValidator_URL "" CACHE INTERNAL "")
		else()
			set(glslangValidator_URL "${glslangValidator_base_URL}/${glslangValidator_PACKAGE_NAME}.zip" CACHE INTERNAL "")
			message("glslangValidator was not found so will be downloaded: ${glslangValidator_URL}")
		endif()
		
		download_external_project("glslangValidator" "${CMAKE_CURRENT_BINARY_DIR}/glslangValidator/download" "${glslang_DOWNLOAD_DIR}/${glslangValidator_PACKAGE_NAME}" 
			"${glslang_DOWNLOAD_DIR}" "${glslangValidator_URL}" "${glslangValidator_BINARY_LOCATION}")
	else()
		export(TARGETS glslangValidator FILE "${CMAKE_CURRENT_BINARY_DIR}/build/glslangValidatorConfig.cmake")
		file(COPY glslangValidatorConfig.cmake.in DESTINATION "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/glslang/" NO_SOURCE_PERMISSIONS)
		configure_file("${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/glslang/glslangValidatorConfig.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/glslang/glslangValidatorConfig.cmake" COPYONLY)
	endif()
	
	# Switch the following when glslang properly export their targets.
	#export(EXPORT OGLCompilerTargets FILE "${CMAKE_CURRENT_BINARY_DIR}/build/OGLCompilerConfig.cmake")
	#export(EXPORT OSDependentTargets FILE "${CMAKE_CURRENT_BINARY_DIR}/build/OSDependentConfig.cmake")
	#export(EXPORT glslangTargets FILE "${CMAKE_CURRENT_BINARY_DIR}/build/glslangConfig.cmake")
	#export(EXPORT SPIRVTargets FILE "${CMAKE_CURRENT_BINARY_DIR}/build/SPIRVConfig.cmake")
	export(TARGETS OGLCompiler FILE "${CMAKE_CURRENT_BINARY_DIR}/build/OGLCompilerConfig.cmake")
	export(TARGETS OSDependent FILE "${CMAKE_CURRENT_BINARY_DIR}/build/OSDependentConfig.cmake")
	export(TARGETS glslang FILE "${CMAKE_CURRENT_BINARY_DIR}/build/glslangConfig.cmake")
	export(TARGETS SPIRV FILE "${CMAKE_CURRENT_BINARY_DIR}/build/SPIRVConfig.cmake")
	
	# build tree package config
	file(COPY OGLCompilerConfig.cmake.in DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/glslang/" NO_SOURCE_PERMISSIONS)
	configure_file("${CMAKE_CURRENT_BINARY_DIR}/glslang/OGLCompilerConfig.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/glslang/OGLCompilerConfig.cmake" COPYONLY)
	file(COPY OSDependentConfig.cmake.in DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/glslang/" NO_SOURCE_PERMISSIONS)
	configure_file("${CMAKE_CURRENT_BINARY_DIR}/glslang/OSDependentConfig.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/glslang/OSDependentConfig.cmake" COPYONLY)
	file(COPY glslangConfig.cmake.in DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/glslang/" NO_SOURCE_PERMISSIONS)
	configure_file("${CMAKE_CURRENT_BINARY_DIR}/glslang/glslangConfig.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/glslang/glslangConfig.cmake" COPYONLY)
	file(COPY SPIRVConfig.cmake.in DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/glslang/" NO_SOURCE_PERMISSIONS)
	configure_file("${CMAKE_CURRENT_BINARY_DIR}/glslang/SPIRVConfig.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/glslang/SPIRVConfig.cmake" COPYONLY)
endif()

if(CMAKE_CROSSCOMPILING OR APPLE OR PVR_GLSLANG_VALIDATOR_INSTALL_DIR)
	# Create an import target for glslangValidator so we can use the target name directly
	add_executable(glslangValidator IMPORTED GLOBAL)
	
	set_target_properties(glslangValidator PROPERTIES
		IMPORTED_LOCATION "${glslangValidator_BINARY_LOCATION}")
endif()