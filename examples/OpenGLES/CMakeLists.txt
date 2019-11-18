cmake_minimum_required(VERSION 3.3)
project(OpenGLESExamples)

if (PVR_BUILD_OPENGLES2_EXAMPLES)
	set(EXAMPLES 
		01_HelloAPI
		02_IntroducingPVRShell
		03_IntroducingPVRUtils
		04_IntroducingUIRenderer
		05_IntroducingPVRCamera
		PVRScopeExample
		PVRScopeRemote
	)
else()
	set(EXAMPLES 
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
		ImageBasedLighting
	)
endif()

foreach(EXAMPLE ${EXAMPLES})
	message ("==>CMake generation for OpenGLES/${EXAMPLE}...")
	add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/${EXAMPLE}")
endforeach()