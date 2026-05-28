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
			message(WARNING "Found glm_DIR: ${glm_DIR}")
        else()
			message(WARNING "Failed to find glm_DIR_GLOB. PVR_ANDROID_BUILD_TYPE=${PVR_ANDROID_BUILD_TYPE}, ANDROID_ABI=${ANDROID_ABI}")
        endif()
	endif()
endif()

message(WARNING "Findglm.cmake loaded from ${CMAKE_CURRENT_LIST_DIR}")
if(NOT TARGET glm)
    message(WARNING "Findglm: Defining glm INTERFACE IMPORTED GLOBAL")
    add_library(glm INTERFACE IMPORTED GLOBAL)
    get_filename_component(GLM_INC_DIR "${CMAKE_CURRENT_LIST_DIR}/../../external/glm/source" ABSOLUTE)
    set_target_properties(glm PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${GLM_INC_DIR}")
else()
    get_target_property(glm_type glm TYPE)
    get_target_property(glm_imported glm IMPORTED)
    message(WARNING "Findglm: TARGET glm ALREADY EXISTS! TYPE: ${glm_type} IMPORTED: ${glm_imported}")
endif()
if(TARGET glm)
    set(glm_FOUND TRUE)
endif()
