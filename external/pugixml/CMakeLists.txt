cmake_minimum_required(VERSION 3.3)

if(NOT PVR_PREBUILT_DEPENDENCIES)
	include(../../cmake/Functions.cmake)

	# Set current pugixml version
	set(external_pugixml_VERSION_NUMBER "1.9" CACHE INTERNAL "")

	# Set the pugixml source directory
	set(pugixml_SRC_DIR ${CMAKE_CURRENT_BINARY_DIR}/src CACHE INTERNAL "")

	# Set pugixml download directory - we use a downloads directory to allow us to maintain cleanliness
	set(pugixml_DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}/downloads/pugixml_downloads CACHE INTERNAL "")

	# Don't download if it is already present - this handles cases where internet connectivity may be limited but all packages are already available
	if(EXISTS ${pugixml_DOWNLOAD_DIR}/pugixml-${external_pugixml_VERSION_NUMBER}.tar.gz AND EXISTS ${pugixml_SRC_DIR}/CMakeLists.txt)
		set(pugixml_URL "" CACHE INTERNAL "")
	else()
		set(pugixml_URL "https://github.com/zeux/pugixml/releases/download/v${external_pugixml_VERSION_NUMBER}/pugixml-${external_pugixml_VERSION_NUMBER}.tar.gz" CACHE INTERNAL "")
		# Remove the existing pugixml downloads so that only the newest version of pugixml will be present
		file(REMOVE_RECURSE ${pugixml_DOWNLOAD_DIR})
		message("pugixml was not found so will be downloaded: ${pugixml_URL}")
	endif()

	download_external_project("pugixml" "${CMAKE_CURRENT_BINARY_DIR}/download" "${pugixml_SRC_DIR}" "${pugixml_DOWNLOAD_DIR}" "${pugixml_URL}" "${pugixml_SRC_DIR}/CMakeLists.txt")

	if(EXISTS ${pugixml_SRC_DIR}/tests/)
		# Remove the tests/ folder from pugixml. The use of filenames with unicode characters cause issues in various places. As we are not currently using pugixml tests we'll remove the tests/ directory.
		file(REMOVE_RECURSE ${pugixml_SRC_DIR}/tests/)
	endif()

	add_subdirectory("${pugixml_SRC_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/build")

	# Use c++14 with no extensions
	set_target_properties(pugixml PROPERTIES CXX_STANDARD 14)

	# Export pugixml from the build tree for use by other projects
	export(EXPORT pugixml-config FILE "${CMAKE_CURRENT_BINARY_DIR}/build/pugixmlConfig.cmake")
	# build tree package config
	file(COPY pugixmlConfig.cmake.in DESTINATION "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/build/" NO_SOURCE_PERMISSIONS)
	configure_file("${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/build/pugixmlConfig.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/build/pugixmlConfig.cmake" COPYONLY)
endif()