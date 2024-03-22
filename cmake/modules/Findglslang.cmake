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
		set(glslang_DIR "${CMAKE_CURRENT_LIST_DIR}/../../external/glslang/build-android/.cxx/cmake/${PVR_ANDROID_BUILD_TYPE}/${ANDROID_ABI}/build")
		set(SPIRV_DIR "${CMAKE_CURRENT_LIST_DIR}/../../external/glslang/build-android/.cxx/cmake/${PVR_ANDROID_BUILD_TYPE}/${ANDROID_ABI}/build")
		set(OGLCompiler_DIR "${CMAKE_CURRENT_LIST_DIR}/../../external/glslang/build-android/.cxx/cmake/${PVR_ANDROID_BUILD_TYPE}/${ANDROID_ABI}/build")
		set(OSDependent_DIR "${CMAKE_CURRENT_LIST_DIR}/../../external/glslang/build-android/.cxx/cmake/${PVR_ANDROID_BUILD_TYPE}/${ANDROID_ABI}/build")
		set(GenericCodeGen_DIR "${CMAKE_CURRENT_LIST_DIR}/../../external/glslang/build-android/.cxx/cmake/${PVR_ANDROID_BUILD_TYPE}/${ANDROID_ABI}/build")
		set(MachineIndependent_DIR "${CMAKE_CURRENT_LIST_DIR}/../../external/glslang/build-android/.cxx/cmake/${PVR_ANDROID_BUILD_TYPE}/${ANDROID_ABI}/build")

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

