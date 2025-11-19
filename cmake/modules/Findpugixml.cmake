# Findpugixml.cmake
#
# Finds the pugixml library and its dependencies
#
# This will define the following imported targets
#     pugixml

if(PVR_PREBUILT_DEPENDENCIES)
	if(ANDROID)
		string(TOLOWER ${CMAKE_BUILD_TYPE} PVR_ANDROID_BUILD_TYPE)
		# The new AGP structure includes a random hash, so we need to use a wildcard to find the correct path.
		file(GLOB pugixml_DIR_GLOB "${CMAKE_CURRENT_LIST_DIR}/../../external/pugixml/build-android/.cxx/${PVR_ANDROID_BUILD_TYPE}/*/${ANDROID_ABI}/build")
		# The glob will return a list, but there should only be one match.
		list(GET pugixml_DIR_GLOB 0 pugixml_DIR)
	endif()
endif()

if(NOT TARGET pugixml)
	find_package(pugixml REQUIRED CONFIG)
endif()