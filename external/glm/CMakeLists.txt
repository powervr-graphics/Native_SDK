cmake_minimum_required(VERSION 3.18.0 FATAL_ERROR)
project(glm)

# Check that glm hasn't been added before
if(TARGET glm)
	return()
endif()

set(GLM_TEST_ENABLE_FAST_MATH ON CACHE INTERNAL "")
set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
set(BUILD_STATIC_LIBS OFF CACHE INTERNAL "")
set(GLM_TEST_ENABLE OFF CACHE INTERNAL "")

# Turn off depreciation warnings
set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "" FORCE)

# GLM provides a module GNUInstallDirs which overrides default CMake behaviour so ensure 
# GNUInstallDirs is included prior to the add_subdirectory call for glm so that the 
# "default" behaviour is observed
include(GNUInstallDirs)
add_subdirectory(source "${CMAKE_CURRENT_BINARY_DIR}/build")