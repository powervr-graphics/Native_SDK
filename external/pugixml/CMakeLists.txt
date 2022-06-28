cmake_minimum_required(VERSION 3.18.0 FATAL_ERROR)
project(pugixml)

# Ensure pugixml is only added once
if(TARGET pugixml)
	return()
endif()

# Ensure that the pugixml tests are not build
# This is because they use unicode and it causes problems
set(BUILD_TESTS OFF CACHE BOOL "Don't test PugiXML")

# Add the pugixml source into the tree
add_subdirectory(source "${CMAKE_CURRENT_BINARY_DIR}/build")