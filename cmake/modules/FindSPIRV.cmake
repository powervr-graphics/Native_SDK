# Findglslang.cmake
#
# Finds the glslang, SPIRV, OGLCompiler, OSDependent targets
#
# This will define the following imported targets
#	  SPIRV

if(PVR_PREBUILT_DEPENDENCIES)
	if(ANDROID)
		string(TOLOWER ${CMAKE_BUILD_TYPE} PVR_ANDROID_BUILD_TYPE)
		#set(SPIRV_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../external/glslang/build-android/.cxx/cmake/${PVR_ANDROID_BUILD_TYPE}/${ANDROID_ABI}/build")


		file(GLOB SPIRV_DIR_GLOB "${CMAKE_CURRENT_LIST_DIR}/../../external/glslang/build-android/.cxx/${CMAKE_BUILD_TYPE}/*/${ANDROID_ABI}/build")
		# The glob will return a list, but there should only be one match.
		list(GET SPIRV_DIR_GLOB 0 SPIRV_DIR)


	endif()
endif()

if(NOT TARGET SPIRV)
	find_package(SPIRV REQUIRED CONFIG)
endif()