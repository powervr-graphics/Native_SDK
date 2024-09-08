cmake_minimum_required(VERSION 3.10)
project(VulkanMatrixMultiplication)
if(IOS)
	message ("Skipping VulkanMatrixMultiplication : Vulkan is not supported on iOS.")
	return()
endif()

add_subdirectory(../../.. ${CMAKE_CURRENT_BINARY_DIR}/sdk)

if(PVR_PREBUILT_DEPENDENCIES)
	find_package(PVRCore REQUIRED MODULE)
	find_package(PVRUtilsVk REQUIRED MODULE)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT VulkanMatrixMultiplication)

set(SRC_FILES VulkanMatrixMultiplication.cpp MatrixMultiplicationGPU.cpp Matrix.cpp MatrixMultiplication.h MatrixMultiplicationGPU.h Matrix.h)

set(ASSET_FOLDER ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Assets_VulkanMatrixMultiplication)

# Adds an executable (or ndk library for Android) and necessary files like plists for Mac/iOS etc.
add_platform_specific_executable(VulkanMatrixMultiplication ${SRC_FILES} COMMAND_LINE)

#################################### ASSET FILES ##########################################
# For platforms supporting it, will be packaged with the executable for runtime use.
# Will be accessible from the app with their "relative path". One call per base path

### Compute shaders (Vulkan) ###
add_assets_to_target(VulkanMatrixMultiplication 
	SOURCE_GROUP "shaders" 
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/
	FILE_LIST 
		MatMulTemplate.csh
		mat_mul_naive_AT.csh
		mat_mul_naive_BT.csh
		mat_mul_naive_CT.csh
		mat_mul_naive_ATCT.csh
		mat_mul_naive_BTCT.csh
		mat_mul_linearwg_AT.csh
		mat_mul_linearwg_BT.csh
		mat_mul_linearwg_vec4.csh
		mat_mul_linearwg_vec4_local.csh
		mat_mul_tile.csh
		mat_mul_tile_vec4.csh
		mat_mul_tile_WF.csh
		mat_mul_rect.csh)

# Create and adds a Windows resource file (Resources.rc) with all the assets that have been added to the target with the previous functions
add_assets_resource_file(VulkanMatrixMultiplication)
###########################################################################################

# Apply SDK example specific compile and linker options
apply_example_compile_options_to_target(VulkanMatrixMultiplication)

target_link_libraries(VulkanMatrixMultiplication PUBLIC
	PVRCore
	PVRUtilsVk
)


#TODO:: REMOVE THIS LATER AFTER THE FRAMEWORK LIBS HAVE BEEN ADDED TO FOR MacOS
###########################################################################################
#For the moment, figure out that this module needs to be added to make the app compile 
#It seems like it's the appkit
if(APPLE)
###########################################################################################
	find_library(lib-AppKit AppKit)
	list(APPEND PLATFORM_LIBS ${lib-AppKit})
	target_link_libraries(VulkanMatrixMultiplication PUBLIC ${PLATFORM_LIBS})
endif()
###########################################################################################
