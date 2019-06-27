cmake_minimum_required(VERSION 3.3)

include(../../cmake/Common.cmake)

# Set current pugixml version
set(external_pugixml_VERSION "v1.9" CACHE INTERNAL "")

# Set pugixml prefix location
set(pugixml_PREFIX ${EXTERNAL_CMAKE_FILES_FOLDER}/external_pugixml CACHE INTERNAL "")

# Set the pugixml source directory
set(pugixml_SRC_DIR ${SDK_ROOT}/external/pugixml/src CACHE INTERNAL "")

# Set pugixml download directory - we use a downloads directory to allow us to maintain cleanliness
set(pugixml_DOWNLOAD_DIR ${SDK_ROOT}/external/downloads/pugixml_downloads CACHE INTERNAL "")

get_directory_property(HAS_PARENT PARENT_DIRECTORY)
if(HAS_PARENT AND USE_PREBUILT_DEPENDENCIES)
	add_library(pugixml STATIC IMPORTED GLOBAL)

	set(pugixml_LIB ${EXTERNAL_LIB_CONFIG_FOLDER}/${CMAKE_STATIC_LIBRARY_PREFIX}pugixml${CMAKE_STATIC_LIBRARY_SUFFIX})

	set_target_properties(pugixml PROPERTIES 
		INTERFACE_INCLUDE_DIRECTORIES "${pugixml_SRC_DIR}/src"
		IMPORTED_LOCATION ${pugixml_LIB})
else()
	# Don't download if it is already present - this handles cases where internet connectivity may be limited but all packages are already available
	if(EXISTS ${pugixml_DOWNLOAD_DIR}/${external_pugixml_VERSION}.tar.gz AND EXISTS ${pugixml_SRC_DIR}/CMakeLists.txt)
		set(pugixml_URL "" CACHE INTERNAL "")
		message("pugixml was found so will not be downloaded: ${pugixml_SRC_DIR}")
	else()
		set(pugixml_URL "https://github.com/zeux/pugixml/archive/${external_pugixml_VERSION}.tar.gz" CACHE INTERNAL "")
		# Remove the existing pugixml downloads so that only the newest version of pugixml will be present
		file(REMOVE_RECURSE ${pugixml_DOWNLOAD_DIR})
		message("pugixml was not found so will be downloaded: ${pugixml_URL}")
	endif()

	download_external_project("pugixml" "${pugixml_PREFIX}" "${pugixml_SRC_DIR}" "${pugixml_DOWNLOAD_DIR}" "${pugixml_URL}" "")

	if(EXISTS ${pugixml_SRC_DIR}/tests/)
		# Remove the tests/ folder from pugixml. The use of filenames with unicode characters cause issues in various places. As we are not currently using pugixml tests we'll remove the tests/ directory.
		file(REMOVE_RECURSE ${pugixml_SRC_DIR}/tests/)
	endif()

	add_subdirectory_if_not_already_included(pugixml "${pugixml_SRC_DIR}" "${pugixml_PREFIX}/build")
		 
	set_target_properties(pugixml PROPERTIES 
		ARCHIVE_OUTPUT_DIRECTORY ${EXTERNAL_LIB_CONFIG_FOLDER}
		LIBRARY_OUTPUT_DIRECTORY ${EXTERNAL_LIB_CONFIG_FOLDER})
endif()
