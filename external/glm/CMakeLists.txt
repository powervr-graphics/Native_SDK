cmake_minimum_required(VERSION 3.10)

project(glm)

if(NOT PVR_PREBUILT_DEPENDENCIES)
	include(../Common.cmake)
	
	# Set the glm source directory
	set(glm_SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/source CACHE INTERNAL "")

	# Set some flags for the glm library
	set(GLM_TEST_ENABLE_FAST_MATH ON CACHE INTERNAL "")
	set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
	set(BUILD_STATIC_LIBS OFF CACHE INTERNAL "")
	set(GLM_TEST_ENABLE OFF CACHE INTERNAL "")

	# GLM provides a module GNUInstallDirs which overrides default CMake behaviour so ensure GNUInstallDirs is included prior to the add_subdirectory call for glm so that the "default" behaviour is observed
	include(GNUInstallDirs)
	add_subdirectory("${glm_SRC_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/build")
endif()
