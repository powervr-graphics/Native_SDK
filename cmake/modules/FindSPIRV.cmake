# FindSPIRV.cmake
#
# Finds the SPIRV target
#
# This will define the following imported targets
#	  SPIRV

if(PVR_PREBUILT_DEPENDENCIES)
	if(ANDROID)
		string(TOLOWER ${CMAKE_BUILD_TYPE} PVR_ANDROID_BUILD_TYPE)
		file(GLOB SPIRV_DIR_GLOB "${CMAKE_CURRENT_LIST_DIR}/../../external/glslang/build-android/.cxx/${CMAKE_BUILD_TYPE}/*/${ANDROID_ABI}")
		file(GLOB SPIRV_DIR_GLOB2 "${CMAKE_CURRENT_LIST_DIR}/../../external/glslang/build-android/.cxx/${PVR_ANDROID_BUILD_TYPE}/*/${ANDROID_ABI}")
		list(APPEND SPIRV_DIR_GLOB ${SPIRV_DIR_GLOB2})

		set(SPIRV_DIR "")
		foreach(DIR IN LISTS SPIRV_DIR_GLOB)
			if(EXISTS "${DIR}/SPIRVTargets.cmake")
				set(SPIRV_DIR "${DIR}")
				break()
			endif()
		endforeach()
	endif()
endif()

if(NOT TARGET SPIRV)
	find_package(SPIRV REQUIRED CONFIG)
endif()
