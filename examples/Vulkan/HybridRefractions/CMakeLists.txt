cmake_minimum_required(VERSION 3.10)
project(VulkanHybridRefractions)
if(ANDROID)
	message("Skipping VulkanHybridRefractions : Not supported on Android")
	return()
endif()
if(IOS)
	message("Skipping VulkanHybridRefractions : Not supported on iOS")
	return()
endif()
if(APPLE AND NOT IOS)
	message("Skipping VulkanHybridRefractions : Not supported on macOS")
	return()
endif()

add_subdirectory(../../.. ${CMAKE_CURRENT_BINARY_DIR}/sdk)

if(PVR_PREBUILT_DEPENDENCIES)
	find_package(PVRShell REQUIRED MODULE)
	find_package(PVRUtilsVk REQUIRED MODULE)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT VulkanHybridRefractions)

set(SRC_FILES VulkanHybridRefractions.cpp)

set(ASSET_FOLDER ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Assets_VulkanHybridRefractions)

# Adds an executable (or ndk library for Android) and necessary files like plists for Mac/iOS etc.
add_platform_specific_executable(VulkanHybridRefractions ${SRC_FILES})

#################################### ASSET FILES ##########################################
# For platforms supporting it, will be packaged with the executable for runtime use.
# Will be accessible from the app with their "relative path". One call per base path

### Textures, scene files, generic resources... ###
add_assets_to_target(VulkanHybridRefractions
	SOURCE_GROUP "assets" 
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Balloons
	FILE_LIST 
		BalloonTex.pvr
		BalloonTex_astc.pvr
		BalloonTex2.pvr
		BalloonTex2_astc.pvr
		BalloonTex3.pvr
		BalloonTex3_astc.pvr
		Balloon.pod)

### Textures, scene files, generic resources... ###
add_assets_to_target(VulkanHybridRefractions
	SOURCE_GROUP "assets" 
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Environments
	FILE_LIST 
		HeroesSquare.pvr
		HeroesSquare_astc.pvr)

### Textures, scene files, generic resources... ###
add_assets_to_target(VulkanHybridRefractions
	SOURCE_GROUP "assets" 
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../assets/RayTracingRefractions
	FILE_LIST 
		Refractions.pod)

### Shaders that need to be compiled to SpirV (Vulkan) ###
# During build time, glslangvalidator will be called to create a "<shadername.extension>.spv" file
add_spirv_shaders_to_target(VulkanHybridRefractions 
	SOURCE_GROUP "shaders_source" 
	SPIRV_SOURCE_GROUP "shaders_generated"
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/
	VULKAN_TARGET_ENV spirv1.4
	FILE_LIST
		DeferredShadingFragmentShader.fsh
		GBufferFragmentShader.fsh
		SeparableGaussianBlurPass.fsh
		FullscreenQuadVertexShader.vsh
		GBufferVertexShader.vsh
		SeparableGaussianBlurPass.vsh
		RayTraceRefractions.rchit
		RayTraceRefractions.rgen
		RayTraceRefractions.rmiss
		RayTraceShadows.rchit
		RayTraceShadows.rmiss)

# Create and adds a Windows resource file (Resources.rc) with all the assets that have been added to the target with the previous functions
add_assets_resource_file(VulkanHybridRefractions)
###########################################################################################

# Apply SDK example specific compile and linker options
apply_example_compile_options_to_target(VulkanHybridRefractions)

target_link_libraries(VulkanHybridRefractions PUBLIC
	PVRShell
	PVRUtilsVk
)
