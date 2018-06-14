cmake_minimum_required(VERSION 3.3)
project(PowerVR_SDK)
include(cmake/Common.cmake)
set (TOP_LEVEL_BUILD 1)

if (NOT BUILD_OPENGLES_EXAMPLES AND NOT BUILD_VULKAN_EXAMPLES)
	if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/framework/PVRUtils/Vulkan/CMakeLists.txt)
		set (BUILD_VULKAN_EXAMPLES 1)
	endif()
	if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/framework/PVRUtils/OpenGLES/CMakeLists.txt)
		set (BUILD_OPENGLES_EXAMPLES 1)
	endif()
endif()

set (OPENGLES_EXAMPLES 
	01_HelloAPI
	02_IntroducingPVRShell
	03_IntroducingPVRUtils
	04_IntroducingUIRenderer
	05_IntroducingPVRCamera
	MultiviewVR
	PostProcessing
	OpenCLExample
	Skinning
	ParticleSystem
	GaussianBlur
	Navigation2D
	Navigation3D
	DeferredShading
	PVRScopeExample
	PVRScopeRemote
	IMGFramebufferDownsample
	IMGTextureFilterCubic
)

set (VULKAN_EXAMPLES 
	01_HelloAPI
	02_IntroducingPVRShell
	03_IntroducingPVRUtils
	04_IntroducingUIRenderer
	Skinning
	MultiSampling
	PostProcessing
	GnomeHorde
	ParticleSystem
	Bumpmap
	GaussianBlur
	Navigation2D
	Navigation3D
	Multithreading
	ExampleUI
	DeferredShading
	DeferredShadingPFX
	Glass
	PVRScopeExample
	PVRScopeRemote
)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/framework/PVRCore ${FRAMEWORK_CMAKE_FILES_FOLDER}/PVRCore)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/framework/PVRAssets ${FRAMEWORK_CMAKE_FILES_FOLDER}/PVRAssets)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/framework/PVRShell ${FRAMEWORK_CMAKE_FILES_FOLDER}/PVRShell)

if (BUILD_OPENGLES_EXAMPLES)
	add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/framework/PVRCamera ${FRAMEWORK_CMAKE_FILES_FOLDER}/PVRCamera)
	add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/framework/PVRUtils/OpenGLES ${FRAMEWORK_CMAKE_FILES_FOLDER}/PVRUtilsGles)
	foreach(EXAMPLE ${OPENGLES_EXAMPLES})
		message ("==>CMake generation for OpenGLES${EXAMPLE}...")
		add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/examples/OpenGLES/${EXAMPLE}")
	endforeach()
endif()

if (BUILD_VULKAN_EXAMPLES)
	add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/framework/PVRVk ${FRAMEWORK_CMAKE_FILES_FOLDER}/PVRVk)
	add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/framework/PVRUtils/Vulkan ${FRAMEWORK_CMAKE_FILES_FOLDER}/PVRUtilsVk)
	foreach(EXAMPLE ${VULKAN_EXAMPLES})
		message ("==>CMake generation for Vulkan${EXAMPLE}...")
		add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/examples/Vulkan/${EXAMPLE}")
	endforeach()
endif()