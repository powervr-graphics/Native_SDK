# Findglslang.cmake
#
# Finds the glslang, SPIRV, OGLCompiler, OSDependent, GenericCodeGen, MachineIndependent targets
#
# This will define the following imported targets
#     glslang
#	  SPIRV
#	  OGLCompiler
#	  OSDependent
#	  GenericCodeGen
#     MachineIndependent

if(PVR_PREBUILT_DEPENDENCIES)
	if(ANDROID)
		string(TOLOWER ${CMAKE_BUILD_TYPE} PVR_ANDROID_BUILD_TYPE)
		#set(glslang_DIR "${CMAKE_CURRENT_LIST_DIR}/../../external/glslang/build-android/.cxx/cmake/${PVR_ANDROID_BUILD_TYPE}/${ANDROID_ABI}/build")
		file(GLOB glslang_DIR_GLOB "${CMAKE_CURRENT_LIST_DIR}/../../external/glslang/build-android/.cxx/${CMAKE_BUILD_TYPE}/*/${ANDROID_ABI}/build")
		# The glob will return a list, but there should only be one match.

		list(GET glslang_DIR_GLOB 0 glslang_DIR)
		list(GET glslang_DIR_GLOB 0 SPIRV_DIR)
		list(GET glslang_DIR_GLOB 0 OGLCompiler_DIR)
		list(GET glslang_DIR_GLOB 0 OSDependent_DIR)
		list(GET glslang_DIR_GLOB 0 GenericCodeGen_DIR)
		list(GET glslang_DIR_GLOB 0 MachineIndependent_DIR)


		# set(OGLCompiler_DIR "${CMAKE_CURRENT_LIST_DIR}/../../external/glslang/build-android/.cxx/cmake/${PVR_ANDROID_BUILD_TYPE}/${ANDROID_ABI}/build")
		# set(OSDependent_DIR "${CMAKE_CURRENT_LIST_DIR}/../../external/glslang/build-android/.cxx/cmake/${PVR_ANDROID_BUILD_TYPE}/${ANDROID_ABI}/build")
		# set(GenericCodeGen_DIR "${CMAKE_CURRENT_LIST_DIR}/../../external/glslang/build-android/.cxx/cmake/${PVR_ANDROID_BUILD_TYPE}/${ANDROID_ABI}/build")
		# set(MachineIndependent_DIR "${CMAKE_CURRENT_LIST_DIR}/../../external/glslang/build-android/.cxx/cmake/${PVR_ANDROID_BUILD_TYPE}/${ANDROID_ABI}/build")

	endif()
endif()

if(NOT TARGET OGLCompiler)
	find_package(OGLCompiler REQUIRED CONFIG)
endif()

if(NOT TARGET OSDependent)
	find_package(OSDependent REQUIRED CONFIG)
endif()

if(NOT TARGET GenericCodeGen)
	find_package(GenericCodeGen REQUIRED CONFIG)
endif()

if(NOT TARGET MachineIndependent)
	find_package(MachineIndependent REQUIRED CONFIG)
endif()

if(NOT TARGET glslang)
	find_package(glslang REQUIRED CONFIG)
endif()

if(NOT TARGET SPIRV)
	find_package(SPIRV REQUIRED CONFIG)
endif()

