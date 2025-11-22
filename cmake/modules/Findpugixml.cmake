# Findpugixml.cmake
#
# Finds the pugixml library and its dependencies
#
# This will define the following imported targets
#     pugixml

if(PVR_PREBUILT_DEPENDENCIES)
	if(ANDROID)
		set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)
		string(TOLOWER ${CMAKE_BUILD_TYPE} PVR_ANDROID_BUILD_TYPE)
		# The new AGP structure includes a random hash, so we need to use a wildcard to find the correct path.
		file(GLOB pugixml_DIR_GLOB "${CMAKE_CURRENT_LIST_DIR}/../../external/pugixml/build-android/.cxx/${CMAKE_BUILD_TYPE}/*/${ANDROID_ABI}/build")
		# The glob will return a list, but there should only be one match.
		if(pugixml_DIR_GLOB)
			list(GET pugixml_DIR_GLOB 0 pugixml_DIR)
		endif()
	endif()
endif()

if(NOT TARGET pugixml)
	if(pugixml_DIR)
		find_package(pugixml REQUIRED CONFIG)
	else()
		# Fallback to building from the bundled source if prebuilt not found
		if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/../../external/pugixml/CMakeLists.txt")
			set(PVR_PREBUILT_DEPENDENCIES_CACHED ${PVR_PREBUILT_DEPENDENCIES})
			set(PVR_PREBUILT_DEPENDENCIES OFF)
			add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../../external/pugixml" "${CMAKE_BINARY_DIR}/external/pugixml")
			set(PVR_PREBUILT_DEPENDENCIES ${PVR_PREBUILT_DEPENDENCIES_CACHED})
		endif()
	endif()
endif()

if(NOT TARGET pugixml)
	find_package(pugixml REQUIRED CONFIG)
endif()