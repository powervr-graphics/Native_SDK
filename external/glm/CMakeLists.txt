cmake_minimum_required(VERSION 3.3)

if(NOT PVR_PREBUILT_DEPENDENCIES)
	include(../../cmake/Functions.cmake)

	# Set current glm version
	set(external_glm_VERSION "0.9.9.4" CACHE INTERNAL "")

	# Set the glm source directory
	set(glm_SRC_DIR ${CMAKE_CURRENT_BINARY_DIR}/src CACHE INTERNAL "")

	# Set glm download directory - we use a downloads directory to allow us to maintain cleanliness
	set(glm_DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}/downloads/glm_downloads CACHE INTERNAL "")

	# Don't download if it is already present - this handles cases where internet connectivity may be limited but all packages are already available
	if(EXISTS ${glm_DOWNLOAD_DIR}/${external_glm_VERSION}.tar.gz AND EXISTS ${glm_SRC_DIR}/CMakeLists.txt)
		set(glm_URL "" CACHE INTERNAL "")
	else()
		set(glm_URL "https://github.com/g-truc/glm/archive/${external_glm_VERSION}.tar.gz" CACHE INTERNAL "")
		# Remove the existing glm downloads so that only the newest version of glm will be present
		file(REMOVE_RECURSE ${glm_DOWNLOAD_DIR})
		message("glm was not found so will be downloaded: ${glm_URL}")
	endif()

	download_external_project("glm" "${CMAKE_CURRENT_BINARY_DIR}/download" "${glm_SRC_DIR}" "${glm_DOWNLOAD_DIR}" "${glm_URL}" "${glm_SRC_DIR}/CMakeLists.txt")

	set(GLM_TEST_ENABLE_FAST_MATH ON CACHE INTERNAL "")
	set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
	set(BUILD_STATIC_LIBS OFF CACHE INTERNAL "")
	set(GLM_TEST_ENABLE OFF CACHE INTERNAL "")

	# GLM provides a module GNUInstallDirs which overrides default CMake behaviour so ensure GNUInstallDirs is included prior to the add_subdirectory call for glm so that the "default" behaviour is observed
	include(GNUInstallDirs)
	add_subdirectory("${glm_SRC_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/build")
endif()