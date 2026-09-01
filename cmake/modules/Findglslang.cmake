# Findglslang.cmake
#
# Finds the glslang and SPIRV targets
#
# This will define the following imported targets
#     glslang
#	  SPIRV

if(PVR_PREBUILT_DEPENDENCIES)
	if(ANDROID)
		string(TOLOWER ${CMAKE_BUILD_TYPE} PVR_ANDROID_BUILD_TYPE)
		file(GLOB glslang_DIR_GLOB "${CMAKE_CURRENT_LIST_DIR}/../../external/glslang/build-android/.cxx/${CMAKE_BUILD_TYPE}/*/${ANDROID_ABI}")
		file(GLOB glslang_DIR_GLOB2 "${CMAKE_CURRENT_LIST_DIR}/../../external/glslang/build-android/.cxx/${PVR_ANDROID_BUILD_TYPE}/*/${ANDROID_ABI}")
		list(APPEND glslang_DIR_GLOB ${glslang_DIR_GLOB2})

		# Iterate over found directories and pick one that has glslangTargets.cmake
		set(glslang_DIR "")
		set(SPIRV_DIR "")
		foreach(DIR IN LISTS glslang_DIR_GLOB)
			if(EXISTS "${DIR}/glslangTargets.cmake" AND EXISTS "${DIR}/SPIRVTargets.cmake")
				set(glslang_DIR "${DIR}")
				set(SPIRV_DIR "${DIR}")
				break()
			endif()
		endforeach()
	endif()
endif()

find_package(Threads REQUIRED)
if(NOT TARGET glslang)
	find_package(glslang REQUIRED CONFIG)
endif()
