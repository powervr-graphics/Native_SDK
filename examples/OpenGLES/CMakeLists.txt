cmake_minimum_required(VERSION 3.18)
project(OpenGLESExamples)

if (PVR_BUILD_OPENGLES2_EXAMPLES)
	set(OPENGLES_EXAMPLES 
	01_HelloAPI
	02_IntroducingPVRShell
	03_IntroducingPVRUtils
	04_IntroducingUIRenderer
	05_IntroducingPVRCamera
	PVRScopeExample
	PVRScopeRemote
	BinaryShaders
	)
else()
	set(OPENGLES_EXAMPLES 
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
	DeferredShading
	PVRScopeExample
	PVRScopeRemote
	IMGFramebufferDownsample
	IMGTextureFilterCubic
	ImageBasedLighting
	GameOfLife
	BinaryShaders
	Multithreading
	Bumpmap
	Shadows
	)
endif()

foreach(EXAMPLE ${OPENGLES_EXAMPLES})
	message ("==>CMake generation for OpenGLES/${EXAMPLE}...")
	add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/${EXAMPLE}")
endforeach()