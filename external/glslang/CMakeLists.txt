cmake_minimum_required(VERSION 3.3)

include(../../cmake/Common.cmake)

# Set current glslang version
set(external_glslang_VERSION "7.11.3113" CACHE INTERNAL "")

# Set glslang prefix location
set(glslang_PREFIX ${EXTERNAL_CMAKE_FILES_FOLDER}/external_glslang CACHE INTERNAL "")

# Set the glslang source directory
set(glslang_SRC_DIR ${SDK_ROOT}/external/glslang/src CACHE INTERNAL "")

# Set glslang download directory - we use a downloads directory to allow us to maintain cleanliness
set(glslang_DOWNLOAD_DIR ${SDK_ROOT}/external/downloads/glslang_downloads CACHE INTERNAL "")

# Set glslangValidator prefix location
set(glslangValidator_PREFIX ${EXTERNAL_CMAKE_FILES_FOLDER}/external_glslangValidator CACHE INTERNAL "")
set(glslangValidator_RELEASE_EXECUTABLE ${EXTERNAL_RELEASE_BIN_FOLDER}/bin/glslangValidator${HOST_EXECUTABLE_SUFFIX} CACHE INTERNAL "")

get_directory_property(HAS_PARENT PARENT_DIRECTORY)
# If a glslang and glslangValidator already exists
if((HAS_PARENT AND USE_PREBUILT_DEPENDENCIES) OR GLSLANG_INSTALL_DIR OR GLSLANG_VALIDATOR_INSTALL_DIR)
	add_library(glslang STATIC IMPORTED GLOBAL)
	add_library(SPIRV STATIC IMPORTED GLOBAL)
	add_library(OGLCompiler STATIC IMPORTED GLOBAL)
	add_library(OSDependent STATIC IMPORTED GLOBAL)
	
	add_executable(glslangValidator IMPORTED GLOBAL)

	if(NOT GLSLANG_INSTALL_DIR)
		set(glslang_INCLUDE_DIRECTORIES ${glslang_SRC_DIR})
		set(glslang_LIB ${EXTERNAL_LIB_CONFIG_FOLDER}/${CMAKE_STATIC_LIBRARY_PREFIX}glslang${glslang_DEBUG_POSTFIX}${CMAKE_STATIC_LIBRARY_SUFFIX})
		set(SPIRV_LIB ${EXTERNAL_LIB_CONFIG_FOLDER}/${CMAKE_STATIC_LIBRARY_PREFIX}SPIRV${glslang_DEBUG_POSTFIX}${CMAKE_STATIC_LIBRARY_SUFFIX})
		set(OGLCompiler_LIB ${EXTERNAL_LIB_CONFIG_FOLDER}/${CMAKE_STATIC_LIBRARY_PREFIX}OGLCompiler${glslang_DEBUG_POSTFIX}${CMAKE_STATIC_LIBRARY_SUFFIX})
		set(OSDependent_LIB ${EXTERNAL_LIB_CONFIG_FOLDER}/${CMAKE_STATIC_LIBRARY_PREFIX}OSDependent${glslang_DEBUG_POSTFIX}${CMAKE_STATIC_LIBRARY_SUFFIX})
	else()
		set(glslang_INCLUDE_DIRECTORIES ${GLSLANG_INSTALL_DIR}/include)
		set(glslang_LIB ${GLSLANG_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}glslang${glslang_DEBUG_POSTFIX}${CMAKE_STATIC_LIBRARY_SUFFIX})
		set(SPIRV_LIB ${GLSLANG_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}SPIRV${glslang_DEBUG_POSTFIX}${CMAKE_STATIC_LIBRARY_SUFFIX})
		set(OGLCompiler_LIB ${GLSLANG_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}OGLCompiler${glslang_DEBUG_POSTFIX}${CMAKE_STATIC_LIBRARY_SUFFIX})
		set(OSDependent_LIB ${GLSLANG_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}OSDependent${glslang_DEBUG_POSTFIX}${CMAKE_STATIC_LIBRARY_SUFFIX})
	endif()

	set_target_properties(glslang PROPERTIES
		INTERFACE_INCLUDE_DIRECTORIES ${glslang_INCLUDE_DIRECTORIES}
		IMPORTED_LOCATION ${glslang_LIB})

	set_target_properties(SPIRV PROPERTIES 
		INTERFACE_INCLUDE_DIRECTORIES ${glslang_INCLUDE_DIRECTORIES}
		IMPORTED_LOCATION ${SPIRV_LIB})

	set_target_properties(OGLCompiler PROPERTIES 
		INTERFACE_INCLUDE_DIRECTORIES ${glslang_INCLUDE_DIRECTORIES}
		IMPORTED_LOCATION ${OGLCompiler_LIB})

	set_target_properties(OSDependent PROPERTIES 
		INTERFACE_INCLUDE_DIRECTORIES ${glslang_INCLUDE_DIRECTORIES}
		IMPORTED_LOCATION ${OSDependent_LIB})
		
	if(GLSLANG_VALIDATOR_INSTALL_DIR)
		set(glslangValidator_RELEASE_EXECUTABLE ${GLSLANG_VALIDATOR_INSTALL_DIR}/bin/glslangValidator${HOST_EXECUTABLE_SUFFIX} CACHE INTERNAL "")
	endif()
	
	set_target_properties(glslangValidator PROPERTIES 
		IMPORTED_LOCATION ${glslangValidator_RELEASE_EXECUTABLE})
else()
	# Don't download if it is already present - this handles cases where internet connectivity may be limited but all packages are already available
	if(EXISTS ${glslang_DOWNLOAD_DIR}/${external_glslang_VERSION}.tar.gz AND EXISTS ${glslang_SRC_DIR}/CMakeLists.txt)
		set(glslang_URL "" CACHE INTERNAL "")
		message("glslang was found so will not be downloaded: ${glslang_SRC_DIR}")
	else()
		set(glslang_URL "https://github.com/KhronosGroup/glslang/archive/${external_glslang_VERSION}.tar.gz" CACHE INTERNAL "")
		# Remove the existing glslang downloads so that only the newest version of glslang will be present
		file(REMOVE_RECURSE ${glslang_DOWNLOAD_DIR})
		message("glslang was not found so will be downloaded: ${glslang_URL}")
	endif()
		
	# glslang
	download_external_project("glslang" "${glslang_PREFIX}" "${glslang_SRC_DIR}" "${glslang_DOWNLOAD_DIR}" "${glslang_URL}" "")

	# These options are used for both glslang and glslangValidator
	set(SKIP_GLSLANG_INSTALL ON CACHE INTERNAL "")
	set(BUILD_TESTING OFF CACHE INTERNAL "")
	set(ENABLE_HLSL OFF CACHE INTERNAL "")

	# If CMAKE_CROSSCOMPILING then do not build glslangValidator using normal add_subdirectory logic
	if(CMAKE_CROSSCOMPILING)
		# See glslangValidator/CMakeLists.txt for how glslangValidator is built when cross-compiling
		set(ENABLE_GLSLANG_BINARIES OFF CACHE INTERNAL "")
	else()
		set(ENABLE_GLSLANG_BINARIES ON CACHE INTERNAL "")
	endif()

	add_subdirectory_if_not_already_included(glslang "${glslang_SRC_DIR}" "${glslang_PREFIX}/build")
		 
	set_target_properties(glslang PROPERTIES 
		ARCHIVE_OUTPUT_DIRECTORY ${EXTERNAL_LIB_CONFIG_FOLDER}
		LIBRARY_OUTPUT_DIRECTORY ${EXTERNAL_LIB_CONFIG_FOLDER})
		
	set_target_properties(SPIRV PROPERTIES 
		ARCHIVE_OUTPUT_DIRECTORY ${EXTERNAL_LIB_CONFIG_FOLDER}
		LIBRARY_OUTPUT_DIRECTORY ${EXTERNAL_LIB_CONFIG_FOLDER})
		
	set_target_properties(OGLCompiler PROPERTIES 
		ARCHIVE_OUTPUT_DIRECTORY ${EXTERNAL_LIB_CONFIG_FOLDER}
		LIBRARY_OUTPUT_DIRECTORY ${EXTERNAL_LIB_CONFIG_FOLDER})
		
	set_target_properties(OSDependent PROPERTIES 
		ARCHIVE_OUTPUT_DIRECTORY ${EXTERNAL_LIB_CONFIG_FOLDER}
		LIBRARY_OUTPUT_DIRECTORY ${EXTERNAL_LIB_CONFIG_FOLDER})	
		
	# If CMAKE_CROSSCOMPILING then we configure glslangValidator separately and import the executable
	if(CMAKE_CROSSCOMPILING)
		# Don't download if it is already present - this handles cases where internet connectivity may be limited but all packages are already available
		if(NOT EXISTS ${glslangValidator_RELEASE_EXECUTABLE})
			set(glslangValidator_base_URL "https://github.com/KhronosGroup/glslang/releases/download/${external_glslang_VERSION}")
			if(CMAKE_HOST_WIN32)
				set(glslangValidator_URL "${glslangValidator_base_URL}/glslang-master-windows-x64-Release.zip" CACHE INTERNAL "")
			elseif(CMAKE_HOST_APPLE)
				set(glslangValidator_URL "${glslangValidator_base_URL}/glslang-master-osx-Release.zip" CACHE INTERNAL "")
			elseif(CMAKE_HOST_UNIX)
				set(glslangValidator_URL "${glslangValidator_base_URL}/glslang-master-linux-Release.zip" CACHE INTERNAL "")
			endif()
			
			message("glslangValidator was not found and we are cross compiling so it will be downloaded: ${glslangValidator_URL}")

			# glslangValidator
			download_external_project("glslangValidator" "${glslangValidator_PREFIX}" "${EXTERNAL_RELEASE_BIN_FOLDER}" "${glslang_DOWNLOAD_DIR}" "${glslangValidator_URL}" "${glslangValidator_RELEASE_EXECUTABLE}")
		endif()

		# Create an import target for glslangValidator so we can use the target name directly
		add_executable(glslangValidator IMPORTED GLOBAL)
		set_target_properties(glslangValidator PROPERTIES
			IMPORTED_LOCATION ${glslangValidator_RELEASE_EXECUTABLE})
	else()
		# Else glslangValidator will be built as part of the standard build
		# Ensure that the glslangValidator runtime output directory matches the installed directory
		set_target_properties(glslangValidator PROPERTIES 
			RUNTIME_OUTPUT_DIRECTORY_DEBUG ${EXTERNAL_DEBUG_BIN_FOLDER}/bin/
			RUNTIME_OUTPUT_DIRECTORY_RELEASE ${EXTERNAL_RELEASE_BIN_FOLDER}/bin/)
	endif()
	if(WIN32)
		set(CMAKE_DEBUG_POSTFIX "" CACHE INTERNAL "Removes the postfix for debug-built libraries which was set by glslang.")
	endif()
endif()
