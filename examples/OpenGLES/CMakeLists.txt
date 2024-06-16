cmake_minimum_required(VERSION 3.10)
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
	AntiAliasing
	BinaryShaders
	Bumpmap
	DeferredShading
	GameOfLife
	GaussianBlur
	ImageBasedLighting
	IMGFramebufferDownsample
	IMGTextureFilterCubic
	Multithreading
	MultiviewVR
	OpenCLExample
	ParticleSystem
	PostProcessing
	PVRScopeExample
	PVRScopeRemote
	Shadows
	Skinning
	)
endif()

foreach(EXAMPLE ${OPENGLES_EXAMPLES})
	message ("==>CMake generation for OpenGLES/${EXAMPLE}...")
	add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/${EXAMPLE}")
endforeach()