cmake_minimum_required(VERSION 3.10)
# Copyright (c) Imagination Technologies Limited.

project(PVRSuperResolution)

include(GNUInstallDirs)
include(CMakeFindDependencyMacro)
include(../FrameworkCommon.cmake)

# Prevent PVRSuperResolution being added multiple times
if(TARGET PVRSuperResolution)
	return()
endif()

message(STATUS "Adding PVRSuperResolution")

# PVRSuperResolution include files
set(PVRSuperResolution_HEADERS
	DynamicMap.h
	FileIO.h
	Log.h
	MathUtil.h
	PostProcessingInitializationData.h
	PostProcessingPass.h
	SupernovaV1Mode1XPass.h
	SupernovaV1Mode2XPassUpscale.h
	SupernovaV1Mode2XPassOutput.h
	SupernovaYUVAConversionPass.h
	SuperResolution.h
	VulkanComputePostProcessingPass.h
	VulkanGraphicsPostProcessingPass.h
	VulkanPostProcessingPass.h
	VulkanResourceAllocator.h
	)

# PVRCore source files
set(PVRSuperResolution_SRC
	FileIO.cpp
	Log.cpp
	MathUtil.cpp
	SuperResolution.cpp
	SupernovaV1Mode1XPass.cpp
	SupernovaV1Mode2XPassUpscale.cpp
	SupernovaV1Mode2XPassOutput.cpp
	SupernovaYUVAConversionPass.cpp
	VulkanComputePostProcessingPass.cpp
	VulkanGraphicsPostProcessingPass.cpp
	VulkanPostProcessingPass.cpp
	VulkanResourceAllocator.cpp
	)

set(PVRSuperResolution_PRIVATE_LINK_LIBS "")

if(ANDROID)
	find_library(lib-android android)
	list(APPEND PVRSuperResolution_PRIVATE_LINK_LIBS ${lib-android})
	list(APPEND PVRSuperResolution_SRC
		${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)
	
	list(APPEND PVRSuperResolution_PRIVATE_INCLUDE_DIRECTORIES ${ANDROID_NDK}/sources/android/native_app_glue)
endif()

# Explicitly add the parent directory to the include path for this directory
include_directories("${CMAKE_CURRENT_LIST_DIR}/..")
include_directories("${CMAKE_CURRENT_LIST_DIR}/../../include")

# Create the library
add_library(PVRSuperResolution STATIC ${PVRSuperResolution_SRC} ${PVRSuperResolution_HEADERS})

target_link_libraries(PVRSuperResolution
	PRIVATE
		${CMAKE_DL_LIBS}
	PRIVATE
		${PVRSuperResolution_PRIVATE_LINK_LIBS})

if(PVR_PREBUILT_DEPENDENCIES)
	list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../../cmake/modules")
	find_dependency(glm MODULE REQUIRED)
endif()

apply_framework_compile_options_to_target(PVRSuperResolution)

target_link_libraries(PVRSuperResolution PUBLIC glm)

#set(PVRSuperResolution_INTERFACE_INCLUDE_DIRECTORIES 
#	${CMAKE_CURRENT_LIST_DIR}/../../include)

#target_include_directories(PVRSuperResolution 
#	PUBLIC 
#		"$<BUILD_INTERFACE:${PVRSuperResolution_INTERFACE_INCLUDE_DIRECTORIES}>")

target_include_directories(PVRSuperResolution 
	PRIVATE 
		$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../>
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
		${PVRSuperResolution_PRIVATE_INCLUDE_DIRECTORIES})

message(STATUS "Adding PVRSuperResolution FLAG 4")

if(PVR_ENABLE_INSTALL)
	install(TARGETS PVRSuperResolution EXPORT PVRSuperResolutionTargets
		ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
		INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
		OPTIONAL)

	foreach(file ${PVRSuperResolution_HEADERS})
		get_filename_component(dir ${file} DIRECTORY)
		install(FILES ${file} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/PVRSuperResolution/${dir})
	endforeach()
	install(EXPORT PVRSuperResolutionTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/PVRSuperResolution OPTIONAL)

	export(EXPORT PVRSuperResolutionTargets FILE "${CMAKE_CURRENT_BINARY_DIR}/PVRSuperResolution/PVRSuperResolutionTargets.cmake")
	# build tree package config
	file(COPY cmake/PVRSuperResolutionConfig.cmake.in DESTINATION "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/PVRSuperResolution/" NO_SOURCE_PERMISSIONS)
	configure_file("${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/PVRSuperResolution/PVRSuperResolutionConfig.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/PVRSuperResolution/PVRSuperResolutionConfig.cmake" COPYONLY)
	install(FILES "${CMAKE_CURRENT_BINARY_DIR}/PVRSuperResolution/PVRSuperResolutionTargets.cmake" DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/PVRSuperResolution OPTIONAL)
endif()
