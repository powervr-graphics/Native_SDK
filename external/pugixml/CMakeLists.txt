cmake_minimum_required(VERSION 3.10)

if(NOT PVR_PREBUILT_DEPENDENCIES)
	include(../Common.cmake)

	# Set source directory
	set(pugixml_SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/source CACHE INTERNAL "")
	
	if(EXISTS ${pugixml_SRC_DIR}/tests/)
		# Remove the tests/ folder from pugixml. The use of filenames with unicode characters cause issues in various places. As we are not currently using pugixml tests we'll remove the tests/ directory.
		file(REMOVE_RECURSE ${pugixml_SRC_DIR}/tests/)
	endif()

	if(ANDROID)
		add_subdirectory("${pugixml_SRC_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/build")
	else()
		add_subdirectory("${pugixml_SRC_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/source")
	endif()

	# Use c++14 with no extensions
	set_target_properties(pugixml PROPERTIES CXX_STANDARD 14)

	# Export pugixml from the build tree for use by other projects
	export(EXPORT pugixml-config FILE "${CMAKE_CURRENT_BINARY_DIR}/build/pugixmlConfig.cmake")
	# build tree package config
	file(COPY pugixmlConfig.cmake.in DESTINATION "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/build/" NO_SOURCE_PERMISSIONS)
	configure_file("${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/build/pugixmlConfig.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/build/pugixmlConfig.cmake" COPYONLY)

	set_property(GLOBAL APPEND PROPERTY PVR_EXTERNAL_TARGETS pugixml)
endif()