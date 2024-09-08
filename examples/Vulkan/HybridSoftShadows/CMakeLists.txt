cmake_minimum_required(VERSION 3.10)
project(VulkanHybridSoftShadows)
if(ANDROID)
	message("Skipping VulkanHybridSoftShadows : Not supported on Android")
	return()
endif()
if(IOS)
	message("Skipping VulkanHybridSoftShadows : Not supported on iOS")
	return()
endif()
if(APPLE AND NOT IOS)
	message("Skipping VulkanHybridSoftShadows : Not supported on macOS")
	return()
endif()
if(IOS)
	message ("Skipping VulkanHybridSoftShadows : Vulkan is not supported on iOS.")
	return()
endif()

add_subdirectory(../../.. ${CMAKE_CURRENT_BINARY_DIR}/sdk)

if(PVR_PREBUILT_DEPENDENCIES)
	find_package(PVRShell REQUIRED MODULE)
	find_package(PVRUtilsVk REQUIRED MODULE)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT VulkanHybridSoftShadows)

set(SRC_FILES VulkanHybridSoftShadows.cpp)

set(ASSET_FOLDER ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Assets_VulkanHybridSoftShadows)

# Adds an executable (or ndk library for Android) and necessary files like plists for Mac/iOS etc.
add_platform_specific_executable(VulkanHybridSoftShadows ${SRC_FILES})

#################################### ASSET FILES ##########################################
# For platforms supporting it, will be packaged with the executable for runtime use.
# Will be accessible from the app with their "relative path". One call per base path

### Textures, scene files, generic resources... ###
add_assets_to_target(VulkanHybridSoftShadows
	SOURCE_GROUP "assets" 
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../assets/HybridSoftShadows
	FILE_LIST 
		SoftShadows.POD
		Bricks.pvr
		Bricks_astc.pvr
		Carpet.pvr
		Carpet_astc.pvr
		Shadow.pvr
		Shadow_astc.pvr)

add_assets_to_target(VulkanHybridSoftShadows
	SOURCE_GROUP "assets" 
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../assets/RayTracingReflections
	FILE_LIST 
		Orange.pvr
		Orange_astc.pvr
		RustGreen.pvr
		RustGreen_astc.pvr)

add_assets_to_target(VulkanHybridSoftShadows
	SOURCE_GROUP "assets" 
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Satyr
	FILE_LIST 
		Marble.pvr
		Marble_astc.pvr)

### Shaders that need to be compiled to SpirV (Vulkan) ###
# During build time, glslangvalidator will be called to create a "<shadername.extension>.spv" file
add_spirv_shaders_to_target(VulkanHybridSoftShadows 
	SOURCE_GROUP "shaders_source" 
	SPIRV_SOURCE_GROUP "shaders_generated"
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/
	VULKAN_TARGET_ENV spirv1.4
	FILE_LIST
		DeferredShadingFragmentShader.fsh
		FullscreenTriangleVertexShader.vsh
		GBufferFragmentShader.fsh
		GBufferVertexShader.vsh)

# Create and adds a Windows resource file (Resources.rc) with all the assets that have been added to the target with the previous functions
add_assets_resource_file(VulkanHybridSoftShadows)
###########################################################################################

# Apply SDK example specific compile and linker options
apply_example_compile_options_to_target(VulkanHybridSoftShadows)

target_link_libraries(VulkanHybridSoftShadows PUBLIC
	PVRShell
	PVRUtilsVk
)