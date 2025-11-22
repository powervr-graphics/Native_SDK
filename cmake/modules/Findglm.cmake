# Findglm.cmake
#
# Finds the glm target
#
# This will define the following imported targets
#     glm

if(PVR_PREBUILT_DEPENDENCIES)
	if(ANDROID)
		set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)
		string(TOLOWER ${CMAKE_BUILD_TYPE} PVR_ANDROID_BUILD_TYPE)
		message(STATUS "Debug findGlm CMAKE_CURRENT_LIST_DIR ${CMAKE_CURRENT_LIST_DIR}")
		message(STATUS "Debug findGlm glob path ${CMAKE_CURRENT_LIST_DIR}/../../external/glm/build-android/.cxx/${CMAKE_BUILD_TYPE}/*/${ANDROID_ABI}/build}")
		# The new AGP structure includes a random hash, so we need to use a wildcard to find the correct path.
		file(GLOB glm_DIR_GLOB "${CMAKE_CURRENT_LIST_DIR}/../../external/glm/build-android/.cxx/${PVR_ANDROID_BUILD_TYPE}/*/${ANDROID_ABI}/build")
		
        # Fallback to uppercase if lowercase fails
        if(NOT glm_DIR_GLOB)
            file(GLOB glm_DIR_GLOB "${CMAKE_CURRENT_LIST_DIR}/../../external/glm/build-android/.cxx/${CMAKE_BUILD_TYPE}/*/${ANDROID_ABI}/build")
        endif()

		# The glob will return a list, but there should only be one match.
		message(STATUS "Debug findGlm glm_DIR_GLOB ${glm_DIR_GLOB}")

        if(glm_DIR_GLOB)
		    list(GET glm_DIR_GLOB 0 glm_DIR)
        endif()
	endif()
endif()

if(NOT TARGET glm)
	if(glm_DIR)
        find_package(glm REQUIRED CONFIG)
    else()
        # Fallback to building from the bundled source if prebuilt not found
        if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/../../external/glm/CMakeLists.txt")
            set(PVR_PREBUILT_DEPENDENCIES_CACHED ${PVR_PREBUILT_DEPENDENCIES})
            set(PVR_PREBUILT_DEPENDENCIES OFF)
            add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../../external/glm" "${CMAKE_BINARY_DIR}/external/glm")
            set(PVR_PREBUILT_DEPENDENCIES ${PVR_PREBUILT_DEPENDENCIES_CACHED})
        endif()
    endif()
endif()

if(NOT TARGET glm)
	find_package(glm REQUIRED CONFIG)
endif()