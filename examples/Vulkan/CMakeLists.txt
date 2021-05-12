cmake_minimum_required(VERSION 3.10)
project(VulkanExamples)

set(VULKAN_EXAMPLES 
	01_HelloAPI
	02_IntroducingPVRShell
	03_IntroducingPVRVk
	04_IntroducingPVRUtils
	05_IntroducingUIRenderer
	Skinning
	MultiSampling
	PostProcessing
	GnomeHorde
	ParticleSystem
	Bumpmap
	GaussianBlur
	Multithreading
	ExampleUI
	DeferredShading
	DeferredShadingPFX
	Glass
	PVRScopeExample
	PVRScopeRemote
	ImageBasedLighting
	IMGTextureFilterCubic
	ImageBasedLightingMapGenerator
	GameOfLife
	PipelineCache
	MatrixMultiplication
	HelloRayTracing
	HybridHardShadows
	HybridReflections
	HybridSoftShadows
)

foreach(EXAMPLE ${VULKAN_EXAMPLES})
	message ("==>CMake generation for Vulkan/${EXAMPLE}...")
	add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/${EXAMPLE}")
endforeach()