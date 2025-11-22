# Findglm.cmake
#
# Finds the glm target
#
# This will define the following imported targets
#     glm

if(PVR_PREBUILT_DEPENDENCIES)
	if(ANDROID)
		set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)
		string(TOLOWER "${CMAKE_BUILD_TYPE}" PVR_ANDROID_BUILD_TYPE)
		message(STATUS "Debug findGlm CMAKE_CURRENT_LIST_DIR ${CMAKE_CURRENT_LIST_DIR}")
		
		# The new AGP structure includes a random hash, so we need to use a wildcard to find the correct path.
		# Try lowercase build type first
		set(SEARCH_PATH "${CMAKE_CURRENT_LIST_DIR}/../../external/glm/build-android/.cxx/${PVR_ANDROID_BUILD_TYPE}/*/${ANDROID_ABI}/build")
		message(STATUS "Debug findGlm glob path ${SEARCH_PATH}")
		file(GLOB glm_DIR_GLOB "${SEARCH_PATH}")

		# If not found, try original build type (case sensitive on Linux)
		if(NOT glm_DIR_GLOB)
			set(SEARCH_PATH "${CMAKE_CURRENT_LIST_DIR}/../../external/glm/build-android/.cxx/${CMAKE_BUILD_TYPE}/*/${ANDROID_ABI}/build")
			message(STATUS "Debug findGlm glob path retry ${SEARCH_PATH}")
			file(GLOB glm_DIR_GLOB "${SEARCH_PATH}")
		endif()

		# The glob will return a list, but there should only be one match.
		message(STATUS "Debug findGlm glm_DIR_GLOB ${glm_DIR_GLOB}")

		list(LENGTH glm_DIR_GLOB LEN)
		if(LEN GREATER 0)
			list(GET glm_DIR_GLOB 0 glm_DIR)
		else()
			message(WARNING "Could not find glm build directory")
		endif()
	endif()
endif()

if(NOT TARGET glm)
	find_package(glm REQUIRED CONFIG)
endif()
