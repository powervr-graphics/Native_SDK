# This file provides all of the information that is required for independent parts of the framework to be able to build 
# through cmake, this primarily focuses on ensuring that all modules know where the root of the SDK is and also 
# how to find different submodules

# Add a cmake function which streamlines adding submodues to the SDK
include(${CMAKE_CURRENT_LIST_DIR}/../cmake/utilities/submodules.cmake)

# Add a cmake function which adds the default framework compile definitions
include(${CMAKE_CURRENT_LIST_DIR}/../cmake/utilities/compile_options.cmake)

if (NOT ("${CMAKE_CURRENT_LIST_DIR}/cmake/modules" IN_LIST CMAKE_MODULE_PATH))
	# CMAKE_MODULE_PATH is used by find_package
	set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../cmake/modules" CACHE STRING "" FORCE)
endif()
