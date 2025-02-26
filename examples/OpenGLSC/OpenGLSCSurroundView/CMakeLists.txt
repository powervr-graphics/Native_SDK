cmake_minimum_required(VERSION 3.3)
project(OpenGLSCSurroundView)

add_subdirectory(../../.. ${CMAKE_CURRENT_BINARY_DIR}/sdk)

if(PVR_PREBUILT_DEPENDENCIES)
	find_package(PVRShell REQUIRED MODULE)
	find_package(PVRUtilsGles REQUIRED MODULE)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT OpenGLSCSurroundView)

set(SRC_FILES OpenGLSCSurroundView.cpp)

set(ASSET_FOLDER ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Assets_OpenGLSCSurroundView)

# Adds an executable (or ndk library for Android) and necessary files like plists for Mac/iOS etc.
add_platform_specific_executable(OpenGLSCSurroundView ${SRC_FILES})

#################################### ASSET FILES ##########################################
# For platforms supporting it, will be packaged with the executable for runtime use.
# Will be accessible from the app with their "relative path". One call per base path

### Textures, scene files, generic resources... ###
add_assets_to_target(OpenGLSCSurroundView
	SOURCE_GROUP "assets" 
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Surroundview
	FILE_LIST
		Environment.bin
		Environment.gltf
		CarFront.pvr
		CarRight.pvr
		CarLeft.pvr
		CarBack.pvr		
		CarBack_astc.pvr
		CarFront_astc.pvr
		CarLeft_astc.pvr
		CarRight_astc.pvr)

add_assets_to_target(OpenGLSCSurroundView
	SOURCE_GROUP "assets" 
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../assets/ToyCar
	FILE_LIST 
		ToyCar.gltf
		ToyCar_basecolor.pvr
		ToyCar_basecolor_astc.pvr
		ToyCar_normal.pvr
		ToyCar_normal_astc.pvr
		ToyCar_roughness_metallic.pvr
		ToyCar_roughness_metallic_astc.pvr)

add_assets_to_target(OpenGLSCSurroundView
	SOURCE_GROUP "assets" 
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../assets/PBR
	FILE_LIST 
		brdfLUT.pvr)

### Textural shaders (OpenGL ES) ###
### OpenGL SC shaders need add manually and compile to binary file before execute example ###
add_assets_to_target(OpenGLSCSurroundView 
	SOURCE_GROUP "shaders" 
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/
	FILE_LIST
		SurroundVertShader.vsh
		SurroundFragShader.fsh
		CarVertShader.vsh
		CarFragShader.fsh
		mipmapPostProcessVertShader.vsh
		mipmapPostProcessFragShader.fsh)

# Create and adds a Windows resource file (Resources.rc) with all the assets that have been added to the target with the previous functions
add_assets_resource_file(OpenGLSCSurroundView)
###########################################################################################

# Apply SDK example specific compile and linker options
apply_example_compile_options_to_target(OpenGLSCSurroundView)

target_link_libraries(OpenGLSCSurroundView PUBLIC
	PVRShell
	PVRUtilsGles
)