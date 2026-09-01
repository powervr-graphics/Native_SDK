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
		file(GLOB tinygltf_DIR_GLOB "${CMAKE_CURRENT_LIST_DIR}/../../external/tinygltf/build-android/.cxx/${PVR_ANDROID_BUILD_TYPE}/*/${ANDROID_ABI}")
		file(GLOB tinygltf_DIR_GLOB2 "${CMAKE_CURRENT_LIST_DIR}/../../external/tinygltf/build-android/.cxx/${CMAKE_BUILD_TYPE}/*/${ANDROID_ABI}")
        list(APPEND tinygltf_DIR_GLOB ${tinygltf_DIR_GLOB2})

		set(tinygltf_DIR "")
		foreach(DIR IN LISTS tinygltf_DIR_GLOB)
			if(EXISTS "${DIR}/tinygltfConfig.cmake" AND EXISTS "${DIR}/tinygltfTargets.cmake")
				set(tinygltf_DIR "${DIR}")
				break()
			endif()
		endforeach()
	endif()
endif()

if(NOT TARGET tinygltf)
    if(tinygltf_DIR)
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
    get_filename_component(TINYGLTF_INC_DIR "${CMAKE_CURRENT_LIST_DIR}/../../external/tinygltf" ABSOLUTE)
    set_property(TARGET tinygltf APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${TINYGLTF_INC_DIR}")
    set(tinygltf_FOUND TRUE)
endif()
