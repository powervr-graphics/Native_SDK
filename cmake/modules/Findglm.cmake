# Findglm.cmake
#
# Finds the glm target
#
# This will define the following imported targets
#     glm

if(PVR_PREBUILT_DEPENDENCIES)
	if(ANDROID)
		string(TOLOWER ${CMAKE_BUILD_TYPE} PVR_ANDROID_BUILD_TYPE)
		message(STATUS "Debug findGlm CMAKE_CURRENT_LIST_DIR ${CMAKE_CURRENT_LIST_DIR}")
		message(STATUS "Debug findGlm glob path ${CMAKE_CURRENT_LIST_DIR}/../../external/glm/build-android/.cxx/${PVR_ANDROID_BUILD_TYPE}/*/${ANDROID_ABI}/build}")
		# The new AGP structure includes a random hash, so we need to use a wildcard to find the correct path.
		file(GLOB glm_DIR_GLOB "${CMAKE_CURRENT_LIST_DIR}/../../external/glm/build-android/.cxx/${PVR_ANDROID_BUILD_TYPE}/*/${ANDROID_ABI}/build")
		# The glob will return a list, but there should only be one match.
		message(STATUS "Debug findGlm glm_DIR_GLOB ${glm_DIR_GLOB}")

		list(GET glm_DIR_GLOB 0 glm_DIR)
	endif()
endif()

if(NOT TARGET glm)
	find_package(glm REQUIRED CONFIG)
endif()
