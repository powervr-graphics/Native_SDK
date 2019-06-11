cmake_minimum_required(VERSION 3.3)
project(OpenGLESPostProcessing)
if(APPLE)
	if(IOS)
		message("Skipping OpenGLESPostProcessing : Not supported on iOS")
		return()
	endif()
endif()
add_subdirectory(../../.. ${CMAKE_CURRENT_BINARY_DIR}/sdk)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT OpenGLESPostProcessing)

set(SRC_FILES OpenGLESPostProcessing.cpp)

set(ASSET_FOLDER ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Assets_OpenGLESPostProcessing)

# ASSET FILES (textures, scene files, textures etc) - Packaged with the executable for runtime use
set(ASSET_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Satyr/MarbleNormalMap.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Satyr/Marble.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/HdrScenes/satara_night_scale_0.305_rgb9e5.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/HdrScenes/satara_night_scale_0.305_rgb9e5_Irradiance.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/HdrScenes/pink_sunrise_rgb9e5.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/HdrScenes/pink_sunrise_rgb9e5_Irradiance.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/HdrScenes/signal_hill_sunrise_scale_0.312_rgb9e5.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/HdrScenes/signal_hill_sunrise_scale_0.312_rgb9e5_Irradiance.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/GaussianBlurFragmentShader.fsh.in
	${CMAKE_CURRENT_SOURCE_DIR}/LinearGaussianBlurVertexShader.vsh.in
	${CMAKE_CURRENT_SOURCE_DIR}/LinearGaussianBlurFragmentShader.fsh.in
	${CMAKE_CURRENT_SOURCE_DIR}/ComputeGaussianBlurHorizontalShader.csh.in
	${CMAKE_CURRENT_SOURCE_DIR}/ComputeGaussianBlurVerticalShader.csh.in
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Satyr/Satyr.pod
	)
list(APPEND SRC_FILES ${ASSET_FILES})
source_group("assets" FILES ${ASSET_FILES})

# TEXTUAL SHADER FILES (OpenGL ES) - Packaged with the executable for runtime use
set(SHADER_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/Downsample2x2FragShader.fsh
	${CMAKE_CURRENT_SOURCE_DIR}/Downsample2x2VertShader.vsh
	${CMAKE_CURRENT_SOURCE_DIR}/Downsample4x4FragShader.fsh
	${CMAKE_CURRENT_SOURCE_DIR}/Downsample4x4VertShader.vsh
	${CMAKE_CURRENT_SOURCE_DIR}/DualFilterDownSampleFragShader.fsh
	${CMAKE_CURRENT_SOURCE_DIR}/DualFilterDownVertShader.vsh
	${CMAKE_CURRENT_SOURCE_DIR}/DualFilterUpSampleFragShader.fsh
	${CMAKE_CURRENT_SOURCE_DIR}/DualFilterUpSampleMergedFinalPassFragShader.fsh
	${CMAKE_CURRENT_SOURCE_DIR}/DualFilterUpVertShader.vsh
	${CMAKE_CURRENT_SOURCE_DIR}/TentFilterFirstUpSampleFragShader.fsh
	${CMAKE_CURRENT_SOURCE_DIR}/TentFilterUpSampleFragShader.fsh
	${CMAKE_CURRENT_SOURCE_DIR}/TentFilterUpSampleMergedFinalPassFragShader.fsh
	${CMAKE_CURRENT_SOURCE_DIR}/TentFilterUpSampleVertShader.vsh
	${CMAKE_CURRENT_SOURCE_DIR}/FragShader.fsh
	${CMAKE_CURRENT_SOURCE_DIR}/KawaseFragShader.fsh
	${CMAKE_CURRENT_SOURCE_DIR}/KawaseVertShader.vsh
	${CMAKE_CURRENT_SOURCE_DIR}/PostBloomFragShader.fsh
	${CMAKE_CURRENT_SOURCE_DIR}/PostBloomVertShader.vsh
	${CMAKE_CURRENT_SOURCE_DIR}/VertShader.vsh
	${CMAKE_CURRENT_SOURCE_DIR}/SkyboxFragShader.fsh
	${CMAKE_CURRENT_SOURCE_DIR}/SkyboxVertShader.vsh
	${CMAKE_CURRENT_SOURCE_DIR}/GaussianVertShader.vsh
	)
list(APPEND SRC_FILES ${SHADER_FILES})
source_group("shaders" FILES ${SHADER_FILES})

set(RESOURCE_FILES ${ASSET_FILES} ${SHADER_FILES})

# Adds Windows resouces.rc, macOS plists etc. For macOS/iOS, also set the opengl dynamic libs in the "frameworks" group
add_platform_specific_resource_files(SRC_FILES RESOURCE_FILES)

# Adds a platform specific executable
add_platform_specific_executable(OpenGLESPostProcessing "${SRC_FILES}" "${RESOURCE_FILES}")

# Copies the assets to our assets folder
add_rule_copy_assets_to_asset_folder("${RESOURCE_FILES}" "${ASSET_FOLDER}")

target_link_libraries(OpenGLESPostProcessing
	PVRShell
	PVRUtilsGles
)