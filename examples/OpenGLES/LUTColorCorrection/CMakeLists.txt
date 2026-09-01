cmake_minimum_required(VERSION 3.22.1)
project(OpenGLESLUTColorCorrection)
set(CMAKE_UNITY_BUILD ON)
if(IOS)
	message("Skipping OpenGLESLUTColorCorrection : Not supported on iOS")
	return()
endif()

add_subdirectory(../../.. ${CMAKE_CURRENT_BINARY_DIR}/sdk)

if(PVR_PREBUILT_DEPENDENCIES)
	find_package(PVRShell REQUIRED MODULE)
	find_package(PVRUtilsGles REQUIRED MODULE)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT OpenGLESLUTColorCorrection)

set(SRC_FILES OpenGLESLUTColorCorrection.cpp)

set(ASSET_FOLDER ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Assets_OpenGLESLUTColorCorrection)

# Adds an executable (or ndk library for Android) and necessary files like plists for Mac/iOS etc.
add_platform_specific_executable(OpenGLESLUTColorCorrection ${SRC_FILES})

#################################### ASSET FILES ##########################################
# For platforms supporting it, will be packaged with the executable for runtime use.
# Will be accessible from the app with their "relative path". One call per base path

### Textures, scene files, generic resources... ###
add_assets_to_target(OpenGLESLUTColorCorrection
	SOURCE_GROUP "assets" 
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Satyr
	FILE_LIST 
		Satyr.pod
		MarbleNormalMap.pvr
		MarbleNormalMap_astc.pvr
		Marble.pvr
		Marble_astc.pvr)
		
add_assets_to_target(OpenGLESLUTColorCorrection
	SOURCE_GROUP "assets" 
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Postprocessing
	FILE_LIST
		lut_cool_16.pvr
		lut_cool_32.pvr
		lut_cool_16_astc.pvr
		lut_cool_32_astc.pvr
		lut_warm_16.pvr
		lut_warm_32.pvr
		lut_warm_16_astc.pvr
		lut_warm_32_astc.pvr)
		
### Textural shaders (OpenGL ES) ###
add_assets_to_target(OpenGLESLUTColorCorrection 
	SOURCE_GROUP "shaders" 
	ASSET_FOLDER ${ASSET_FOLDER} 
	BASE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/
	FILE_LIST 
		FragShader_ES3.fsh
		VertShader_ES3.vsh
		FragShader_PostProcessing.fsh
		VertShader_PostProcessing.vsh
		FragShader_LUTConvert.fsh)

# Create and adds a Windows resource file (Resources.rc) with all the assets that have been added to the target with the previous functions
add_assets_resource_file(OpenGLESLUTColorCorrection)
###########################################################################################

# Apply SDK example specific compile and linker options
apply_example_compile_options_to_target(OpenGLESLUTColorCorrection)

target_link_libraries(OpenGLESLUTColorCorrection PUBLIC
	PVRShell
	PVRUtilsGles
)