cmake_minimum_required(VERSION 3.3)
project(VulkanPostProcessing)
if(APPLE)
	message ("Vulkan is not supported on Apple platforms.")
	return()
endif()
add_subdirectory(../../.. ${CMAKE_CURRENT_BINARY_DIR}/sdk)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT VulkanPostProcessing)

set(SRC_FILES VulkanPostProcessing.cpp)

set(ASSET_FOLDER ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Assets_VulkanPostProcessing)

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

# SHADER SOURCE FILES (Vulkan) - Compiled to spir-v at build-time
set(SHADER_NAMES
Downsample4x4FragShader.fsh
Downsample4x4VertShader.vsh
DualFilterDownSampleFragShader.fsh
DualFilterDownVertShader.vsh
DualFilterUpSampleFragShader.fsh
DualFilterUpSampleMergedFinalPassFragShader.fsh
DualFilterUpVertShader.vsh
TentFilterFirstUpSampleFragShader.fsh
TentFilterUpSampleFragShader.fsh
TentFilterUpSampleMergedFinalPassFragShader.fsh
TentFilterUpSampleVertShader.vsh
FragShader.fsh
KawaseFragShader.fsh
KawaseVertShader.vsh
PostBloomFragShader.fsh
PostBloomVertShader.vsh
VertShader.vsh
SkyboxFragShader.fsh
SkyboxVertShader.vsh
GaussianVertShader.vsh
	)

foreach (SHADER_FILE ${SHADER_NAMES})
	list(APPEND SHADER_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${SHADER_FILE}")
	list(APPEND SHADER_GENERATED_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${SHADER_FILE}.spv")
endforeach()
list(APPEND SRC_FILES ${SHADER_SOURCE_FILES})
list(APPEND SRC_FILES ${SHADER_GENERATED_FILES})
source_group("shaders_source" FILES ${SHADER_SOURCE_FILES})
source_group("shaders_generated" FILES ${SHADER_GENERATED_FILES})

set(RESOURCE_FILES ${ASSET_FILES} ${SHADER_SOURCE_FILES} ${SHADER_GENERATED_FILES})

# Adds Windows resouces.rc, macOS plists etc. For macOS/iOS, also set the opengl dynamic libs in the "frameworks" group
add_platform_specific_resource_files(SRC_FILES RESOURCE_FILES)

# Generate a pre-build step for compiling glsl shaders to spir-v
add_rule_generate_spirv_from_shaders("${SHADER_NAMES}")

# Adds a platform specific executable
add_platform_specific_executable(VulkanPostProcessing "${SRC_FILES}" "${RESOURCE_FILES}")

# Copies the assets to our assets folder
add_rule_copy_assets_to_asset_folder("${RESOURCE_FILES}" "${ASSET_FOLDER}")

target_link_libraries(VulkanPostProcessing
	PVRShell
	PVRUtilsVk
)