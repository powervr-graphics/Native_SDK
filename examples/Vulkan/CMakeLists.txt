cmake_minimum_required(VERSION 3.22.1)
project(VulkanExamples)
set(CMAKE_UNITY_BUILD ON)

set(VULKAN_EXAMPLES 
	01_HelloAPI
	02_IntroducingPVRShell
	03_IntroducingPVRVk
	04_IntroducingPVRUtils
	05_IntroducingUIRenderer
	AntiAliasing
	AmbientOcclusion
	Bumpmap
	DeferredShading
	DeferredShadingPFX
	ExampleUI
	GameOfLife
	GaussianBlur
	Glass
	GnomeHorde
	GPUControlledRendering
	HelloRayTracing
	HybridHardShadows
	HybridReflections
	HybridRefractions
	HybridSoftShadows
	HybridTransparency
	ImageBasedLighting
	ImageBasedLightingMapGenerator
	IMGTextureFilterCubic
	LUTColorCorrection
	MatrixMultiplication
	MultiSampling
	Multithreading
	NeuralSuperResolution
	ParticleSystem
	PipelineCache
	PostProcessing
	PVRScopeExample
	PVRScopeRemote
	RayTracedHardShadows
	RayTracingDenoising
	Shadows
	Skinning
	SPHFluidSimulation
	Subgroups
	Supernova
	YCbCrTextureSampling
	TimelineSemaphores
)

foreach(EXAMPLE ${VULKAN_EXAMPLES})
	message ("==>CMake generation for Vulkan/${EXAMPLE}...")
	add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/${EXAMPLE}")
endforeach()