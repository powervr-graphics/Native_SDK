cmake_minimum_required(VERSION 3.10)
project(OpenGLESShadows)
if(IOS)
	message("Skipping OpenGLESShadows : Not supported on iOS")
	return()
endif()

add_subdirectory(../../.. ${CMAKE_CURRENT_BINARY_DIR}/sdk)

if(PVR_PREBUILT_DEPENDENCIES)
	find_package(PVRShell REQUIRED MODULE)
	find_package(PVRUtilsGles REQUIRED MODULE)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT OpenGLESShadows)

set(SRC_FILES OpenGLESShadows.cpp)

set(ASSET_FOLDER ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Assets_OpenGLESShadows)

# Adds an executable (or ndk library for Android) and necessary files like plists for Mac/iOS etc.
add_platform_specific_executable(OpenGLESShadows ${SRC_FILES})

#################################### ASSET FILES ##########################################
# For platforms supporting it, will be packaged with the executable for runtime use.
# Will be accessible from the app with their "relative path". One call per base path

### Textures, scene files, generic resources... ###
add_assets_to_target(OpenGLESShadows
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

### Textural shaders (OpenGL ES) ###
add_assets_to_target(OpenGLESShadows 
	SOURCE_GROUP "shaders" 
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/
	FILE_LIST 
		AmbientFragShader.fsh
		GaussianBlurHorizontalFragShader.fsh
		GaussianBlurVerticalFragShader.fsh
		LightingFragShader.fsh
		MeshFragShader.fsh
		ShadowFragShader.fsh
		MeshVertShader.vsh
		ShadowVertShader.vsh
		TriangleVertShader.vsh)

# Create and adds a Windows resource file (Resources.rc) with all the assets that have been added to the target with the previous functions
add_assets_resource_file(OpenGLESShadows)
###########################################################################################

# Apply SDK example specific compile and linker options
apply_example_compile_options_to_target(OpenGLESShadows)

target_link_libraries(OpenGLESShadows PUBLIC
	PVRShell
	PVRUtilsGles
)