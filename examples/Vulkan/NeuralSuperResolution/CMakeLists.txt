cmake_minimum_required(VERSION 3.10)
project(VulkanNeuralSuperResolution)
if(IOS)
	message ("Skipping VulkanNeuralSuperResolution : Vulkan is not supported on iOS.")
	return()
endif()

add_subdirectory(../../.. ${CMAKE_CURRENT_BINARY_DIR}/sdk)

if(PVR_PREBUILT_DEPENDENCIES)
	find_package(PVRShell REQUIRED MODULE)
	find_package(PVRUtilsVk REQUIRED MODULE)
	find_package(PVRSuperResolution REQUIRED MODULE)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT VulkanNeuralSuperResolution)

set(SRC_FILES VulkanNeuralSuperResolution.cpp)

set(ASSET_FOLDER ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Assets_VulkanNeuralSuperResolution)

# Adds an executable (or ndk library for Android) and necessary files like plists for Mac/iOS etc.
add_platform_specific_executable(VulkanNeuralSuperResolution ${SRC_FILES})

#################################### ASSET FILES ##########################################
# For platforms supporting it, will be packaged with the executable for runtime use.
# Will be accessible from the app with their "relative path". One call per base path

### Textures, scene files, generic resources... ###
add_assets_to_target(VulkanNeuralSuperResolution
	SOURCE_GROUP "assets" 
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../assets/damagedHelmet
	FILE_LIST 
		damagedHelmet.bin
		damagedHelmet.gltf
		Default_albedo.pvr
		Default_albedo_astc.pvr
		Default_emissive.pvr
		Default_emissive_astc.pvr
		Default_occlusionRoughnessMetallic.pvr
		Default_occlusionRoughnessMetallic_astc.pvr
		Default_normal.pvr
		Default_normal_astc.pvr)

add_assets_to_target(VulkanNeuralSuperResolution
	SOURCE_GROUP "assets" 
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Environments
	FILE_LIST 
		quarry_r9g9b9e5.pvr
		quarry_r9g9b9e5_Irradiance.pvr
		quarry_r9g9b9e5_Prefiltered.pvr)

add_assets_to_target(VulkanNeuralSuperResolution
	SOURCE_GROUP "assets" 
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../assets/PBR
	FILE_LIST 
		brdfLUT.pvr)

### Shaders that need to be compiled to SpirV (Vulkan) ###
# During build time, glslangvalidator will be called to create a "<shadername.extension>.spv" file
add_spirv_shaders_to_target(VulkanNeuralSuperResolution 
	SOURCE_GROUP "shaders_source" 
	SPIRV_SOURCE_GROUP "shaders_generated"
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/
	VULKAN_TARGET_ENV vulkan1.3
	VULKAN_TARGET_ENV spirv1.3
	FILE_LIST
		PBRFragShader.fsh
		SkyboxFragShader.fsh
		SkyboxVertShader.vsh
		VertShader.vsh
		MentisV2NeuralSuperResolution.csh)

# Create and adds a Windows resource file (Resources.rc) with all the assets that have been added to the target with the previous functions
add_assets_resource_file(VulkanNeuralSuperResolution)
###########################################################################################

# Apply SDK example specific compile and linker options
apply_example_compile_options_to_target(VulkanNeuralSuperResolution)

target_link_libraries(VulkanNeuralSuperResolution PUBLIC
	PVRShell
	PVRUtilsVk
	PVRSuperResolution
)