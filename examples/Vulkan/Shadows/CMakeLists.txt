cmake_minimum_required(VERSION 3.10)
project(VulkanShadows)
if(IOS)
	message ("Skipping VulkanShadows : Vulkan is not supported on iOS.")
	return()
endif()

add_subdirectory(../../.. ${CMAKE_CURRENT_BINARY_DIR}/sdk)

if(PVR_PREBUILT_DEPENDENCIES)
	find_package(PVRShell REQUIRED MODULE)
	find_package(PVRUtilsVk REQUIRED MODULE)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT VulkanShadows)

set(SRC_FILES VulkanShadows.cpp)

set(ASSET_FOLDER ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Assets_VulkanShadows)

# Adds an executable (or ndk library for Android) and necessary files like plists for Mac/iOS etc.
add_platform_specific_executable(VulkanShadows ${SRC_FILES})

#################################### ASSET FILES ##########################################
# For platforms supporting it, will be packaged with the executable for runtime use.
# Will be accessible from the app with their "relative path". One call per base path

### Textures, scene files, generic resources... ###
add_assets_to_target(VulkanShadows
	SOURCE_GROUP "assets" 
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../assets/GnomeToy
	FILE_LIST 
		GnomeToy.pod
		GnomeWood.pvr
		GnomeWood_astc.pvr
		DarkWood.pvr
		DarkWood_astc.pvr
		ClearWood.pvr
		ClearWood_astc.pvr)

### Shaders that need to be compiled to SpirV (Vulkan) ###
# During build time, glslangvalidator will be called to create a "<shadername.extension>.spv" file
add_spirv_shaders_to_target(VulkanShadows 
	SOURCE_GROUP "shaders_source" 
	SPIRV_SOURCE_GROUP "shaders_generated"
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/
	VULKAN_TARGET_ENV vulkan1.0
	FILE_LIST
		GaussianBlurHorizontalEVSM2FragShader.fsh
		GaussianBlurHorizontalEVSM4FragShader.fsh
		GaussianBlurHorizontalVSMFragShader.fsh
		GaussianBlurVerticalFragShader.fsh
		MeshFragShader.fsh
		MeshFragShaderEVSM2.fsh
		MeshFragShaderEVSM4.fsh
		MeshFragShaderHard.fsh
		MeshFragShaderPCFOptimised2x2.fsh
		MeshFragShaderPCFOptimised3x3.fsh
		MeshFragShaderPCFOptimised5x5.fsh
		MeshFragShaderPCFOptimised7x7.fsh
		MeshFragShaderPCFPoissonDisk.fsh
		MeshFragShaderVSM.fsh
		MeshVertShader.vsh
		ShadowFragShader.fsh
		ShadowVertShader.vsh
		TriangleVertShader.vsh
		GaussianBlurEVSM2CompShader.csh
		GaussianBlurEVSM4CompShader.csh
		GaussianBlurVSMCompShader.csh)

# Create and adds a Windows resource file (Resources.rc) with all the assets that have been added to the target with the previous functions
add_assets_resource_file(VulkanShadows)
###########################################################################################

# Apply SDK example specific compile and linker options
apply_example_compile_options_to_target(VulkanShadows)

target_link_libraries(VulkanShadows PUBLIC
	PVRShell
	PVRUtilsVk
)