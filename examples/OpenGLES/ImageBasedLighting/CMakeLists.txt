cmake_minimum_required(VERSION 3.3)
project(OpenGLESImageBasedLighting)
if(APPLE)
	if(IOS)
		message("Skipping OpenGLESImageBasedLighting : Not supported on iOS")
		return()
	endif()
endif()
add_subdirectory(../../.. ${CMAKE_CURRENT_BINARY_DIR}/sdk)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT OpenGLESImageBasedLighting)

set(SRC_FILES OpenGLESImageBasedLighting.cpp)

set(ASSET_FOLDER ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Assets_OpenGLESImageBasedLighting)

# ASSET FILES (textures, scene files, textures etc) - Packaged with the executable for runtime use
set(ASSET_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/damagedHelmet/Default_albedo.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/damagedHelmet/Default_emissive.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/damagedHelmet/Default_occlusionRoughnessMetallic.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/damagedHelmet/Default_normal.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/HdrScenes/misty_pines_rgb9e5.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/HdrScenes/misty_pines_rgb9e5_Irradiance.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/HdrScenes/misty_pines_rgb9e5_Prefiltered.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/HdrScenes/satara_night_scale_0.305_rgb9e5.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/HdrScenes/satara_night_scale_0.305_rgb9e5_Irradiance.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/HdrScenes/satara_night_scale_0.305_rgb9e5_Prefiltered.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/brdfLUT.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/damagedHelmet/damagedHelmet.bin
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/damagedHelmet/damagedHelmet.gltf
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/ParticleSystem/sphere.pod
	)
list(APPEND SRC_FILES ${ASSET_FILES})
source_group("assets" FILES ${ASSET_FILES})

# TEXTUAL SHADER FILES (OpenGL ES) - Packaged with the executable for runtime use
set(SHADER_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/PBRFragShader.fsh
	${CMAKE_CURRENT_SOURCE_DIR}/PBRVertShader.vsh
	${CMAKE_CURRENT_SOURCE_DIR}/SkyboxFragShader.fsh
	${CMAKE_CURRENT_SOURCE_DIR}/SkyboxVertShader.vsh
	)
list(APPEND SRC_FILES ${SHADER_FILES})
source_group("shaders" FILES ${SHADER_FILES})

set(RESOURCE_FILES ${ASSET_FILES} ${SHADER_FILES})

# Adds Windows resouces.rc, macOS plists etc. For macOS/iOS, also set the opengl dynamic libs in the "frameworks" group
add_platform_specific_resource_files(SRC_FILES RESOURCE_FILES)

# Adds a platform specific executable
add_platform_specific_executable(OpenGLESImageBasedLighting "${SRC_FILES}" "${RESOURCE_FILES}")

# Copies the assets to our assets folder
add_rule_copy_assets_to_asset_folder("${RESOURCE_FILES}" "${ASSET_FOLDER}")

target_link_libraries(OpenGLESImageBasedLighting
	PVRShell
	PVRUtilsGles
)