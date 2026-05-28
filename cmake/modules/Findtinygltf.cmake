# Findtinygltf.cmake
#
# Finds the tinygltf target
#
# This will define the following imported targets
#     tinygltf

if(PVR_PREBUILT_DEPENDENCIES)
	if(ANDROID)
		# Allow finding packages in the host file system
		set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)

        string(TOLOWER ${CMAKE_BUILD_TYPE} PVR_ANDROID_BUILD_TYPE)
		# Use wildcard for build type to handle Debug/debug casing differences
		file(GLOB tinygltf_DIR_GLOB "${CMAKE_CURRENT_LIST_DIR}/../../external/tinygltf/build-android/.cxx/${PVR_ANDROID_BUILD_TYPE}/*/${ANDROID_ABI}/tinygltf")

        if(NOT tinygltf_DIR_GLOB)
            file(GLOB tinygltf_DIR_GLOB "${CMAKE_CURRENT_LIST_DIR}/../../external/tinygltf/build-android/.cxx/${CMAKE_BUILD_TYPE}/*/${ANDROID_ABI}/tinygltf")
        endif()

		# The glob will return a list, but there should only be one match.
		if(tinygltf_DIR_GLOB)
			list(GET tinygltf_DIR_GLOB 0 tinygltf_DIR)
		else()
			message(STATUS "tinygltf: No build directory found matching ${CMAKE_CURRENT_LIST_DIR}/../../external/tinygltf/build-android/.cxx/${CMAKE_BUILD_TYPE}/*/${ANDROID_ABI}/tinygltf")
		endif()
	endif()
endif()

if(NOT TARGET tinygltf)
    if(tinygltf_DIR AND EXISTS "${tinygltf_DIR}/tinygltfConfig.cmake")
	    find_package(tinygltf REQUIRED CONFIG)
    else()
        if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/../../external/tinygltf/CMakeLists.txt")
            set(PVR_PREBUILT_DEPENDENCIES_CACHED ${PVR_PREBUILT_DEPENDENCIES})
            set(PVR_PREBUILT_DEPENDENCIES OFF)
            add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../../external/tinygltf" "${CMAKE_BINARY_DIR}/external/tinygltf")
            set(PVR_PREBUILT_DEPENDENCIES ${PVR_PREBUILT_DEPENDENCIES_CACHED})
        endif()
    endif()
endif()
if(TARGET tinygltf)
    set(tinygltf_FOUND TRUE)
endif()
