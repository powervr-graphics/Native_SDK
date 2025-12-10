# Helper function to find prebuilt Android artifacts in .cxx directories
#
# Usage:
#   pvr_find_android_build_path(RESULT_VAR BUILD_ANDROID_DIR TARGET_NAME)
#
# This will search for the target output directory within the standard AGP .cxx structure.
# It sets RESULT_VAR to the path found, or leaves it undefined if not found.

function(pvr_find_android_build_path OUT_VAR BUILD_ANDROID_DIR TARGET_NAME)
    if(NOT ANDROID)
        return()
    endif()

    set(POSSIBLE_PATHS)
    
    # 1. Try with CMAKE_BUILD_TYPE if set
    if(CMAKE_BUILD_TYPE)
        # Exact match
        list(APPEND POSSIBLE_PATHS "${BUILD_ANDROID_DIR}/.cxx/${CMAKE_BUILD_TYPE}/*/${ANDROID_ABI}/${TARGET_NAME}")
        
        # Lowercase match (often AGP uses 'debug' while CMake uses 'Debug')
        string(TOLOWER "${CMAKE_BUILD_TYPE}" TYPE_LOWER)
        if(NOT "${TYPE_LOWER}" STREQUAL "${CMAKE_BUILD_TYPE}")
             list(APPEND POSSIBLE_PATHS "${BUILD_ANDROID_DIR}/.cxx/${TYPE_LOWER}/*/${ANDROID_ABI}/${TARGET_NAME}")
        endif()
    endif()

    # 2. Try wildcard for build type (fallback)
    list(APPEND POSSIBLE_PATHS "${BUILD_ANDROID_DIR}/.cxx/*/*/${ANDROID_ABI}/${TARGET_NAME}")

    # 3. Try variant with 'cmake' subdir (older AGP or specific configurations)
    list(APPEND POSSIBLE_PATHS "${BUILD_ANDROID_DIR}/.cxx/*/*/${ANDROID_ABI}/cmake/${TARGET_NAME}")

    foreach(SEARCH_PATTERN ${POSSIBLE_PATHS})
        file(GLOB FOUND_PATHS "${SEARCH_PATTERN}")
        if(FOUND_PATHS)
            # Pick the first match (most recent or random if multiple hashes)
            list(GET FOUND_PATHS 0 SELECTED_PATH)
            set(${OUT_VAR} "${SELECTED_PATH}" PARENT_SCOPE)
            message(STATUS "Found prebuilt ${TARGET_NAME}: ${SELECTED_PATH}")
            return()
        endif()
    endforeach()

    message(STATUS "Could not find prebuilt ${TARGET_NAME}. Searched in:")
    foreach(pattern ${POSSIBLE_PATHS})
        message(STATUS "  ${pattern}")
    endforeach()
endfunction()
