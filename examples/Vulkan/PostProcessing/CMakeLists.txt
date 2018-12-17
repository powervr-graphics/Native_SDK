cmake_minimum_required(VERSION 3.3)

project(VulkanPostProcessing)

set (VULKAN_EXAMPLE 1)

#Include common functionality.  (Common.cmake)
# Sets up variables ( ${PROJECT_ARCH}, ${SDK_ROOT},${EXTERNAL_LIB_FOLDER}), sets up some defaults (e.g. CMAKE_BUILD_TYPE),sets up the include folders,
# sets up necessary libraries in EXTRA_LIBS (like dynamic linking, Android libaries, X11/xcb/Wayland etc. for Linux), 
# sets up some compilation flags (e.g. link time code generation, disables some warnings which hit on system files etc.)
include (../../../cmake/Common.cmake)

if(APPLE)
	message ("Vulkan is not supported on Apple platforms.")
	return()
endif()

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT VulkanPostProcessing)

set (SRC_FILES VulkanPostProcessing.cpp)

set (ASSET_FOLDER ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Assets_VulkanPostProcessing)

# ASSET FILES (textures, scene files, textures etc) - Packaged with the executable for runtime use
set (ASSET_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Satyr/MarbleNormalMap.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Satyr/Marble.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/MonValley/MonValley_baked_lightmap.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/MonValley/DiffuseIrradianceMap.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Satyr/Satyr.pod
	)
list (APPEND SRC_FILES ${ASSET_FILES})
source_group("assets" FILES ${ASSET_FILES})

# SHADER SOURCE FILES (Vulkan) - Compiled to spir-v at build-time
set (SHADER_NAMES
Downsample2x2FragShader.fsh
Downsample4x4FragShader.fsh
Downsample2x2VertShader.vsh
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
GaussianCompHorizontalShader16f.csh
GaussianCompHorizontalShader16frgba.csh
GaussianCompVerticalShader16f.csh
GaussianCompVerticalShader16frgba.csh
GaussianHorizontalFragShader.fsh
GaussianVerticalFragShader.fsh
GaussianVertShader.vsh
KawaseFragShader.fsh
KawaseVertShader.vsh
LinearGaussianEvenSamplesFragShader.fsh
LinearGaussianEvenSamplesHorizontalVertShader.vsh
LinearGaussianEvenSamplesVerticalVertShader.vsh
LinearGaussianOddSamplesFragShader.fsh
LinearGaussianOddSamplesHorizontalVertShader.vsh
LinearGaussianOddSamplesVerticalVertShader.vsh
PostBloomFragShader.fsh
PostBloomVertShader.vsh
VertShader.vsh
SkyboxFragShader.fsh
SkyboxVertShader.vsh
	)

foreach (SHADER_FILE ${SHADER_NAMES})
	list(APPEND SHADER_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${SHADER_FILE}")
	list(APPEND SHADER_GENERATED_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${SHADER_FILE}.spv")
endforeach()
list (APPEND SRC_FILES ${SHADER_SOURCE_FILES})
list (APPEND SRC_FILES ${SHADER_GENERATED_FILES})
source_group("shaders_source" FILES ${SHADER_SOURCE_FILES})
source_group("shaders_generated" FILES ${SHADER_GENERATED_FILES})

set (RESOURCE_FILES ${ASSET_FILES} ${SHADER_FILES} ${SHADER_GENERATED_FILES})

# Adds Windows resouces.rc, macOS plists etc. For macOS/iOS, also set the opengl dynamic libs in the "frameworks" group
add_platform_specific_resource_files("SRC_FILES" "RESOURCE_FILES")

#GENERATE A PRE-BUILD STEP FOR COMPILING GLSL TO SPIRV
add_rule_generate_spirv_from_shaders("${SHADER_NAMES}")

if (WIN32)
	add_executable( VulkanPostProcessing WIN32 ${SRC_FILES})
elseif (ANDROID)
	add_library( VulkanPostProcessing SHARED ${SRC_FILES})

elseif (UNIX OR QNX)
	add_executable( VulkanPostProcessing ${SRC_FILES})
endif()

add_rule_copy_assets_to_asset_folder("${RESOURCE_FILES}" "${ASSET_FOLDER}")

# Add the Framework subprojects.
if (NOT ANDROID) # Android builds the Framework libraries by using Gradle to kick CMake (this is an Android thing...)
	add_subdirectory_if_not_already_included(PVRCore ${SDK_ROOT}/framework/PVRCore ${FRAMEWORK_CMAKE_FILES_FOLDER}/PVRCore)
	add_subdirectory_if_not_already_included(PVRAssets ${SDK_ROOT}/framework/PVRAssets ${FRAMEWORK_CMAKE_FILES_FOLDER}/PVRAssets)
	add_subdirectory_if_not_already_included(PVRVk ${SDK_ROOT}/framework/PVRVk ${FRAMEWORK_CMAKE_FILES_FOLDER}/PVRVk)
	add_subdirectory_if_not_already_included(PVRShell ${SDK_ROOT}/framework/PVRShell ${FRAMEWORK_CMAKE_FILES_FOLDER}/PVRShell)
	add_subdirectory_if_not_already_included(PVRUtilsVk ${SDK_ROOT}/framework/PVRUtils/Vulkan ${FRAMEWORK_CMAKE_FILES_FOLDER}/PVRUtilsVk)

	add_dependencies(VulkanPostProcessing PVRCore  PVRAssets  PVRVk  PVRShell  PVRUtilsVk) #... We should still add the dependencies, build or not.
endif()

target_link_libraries(VulkanPostProcessing
${FRAMEWORK_LIB_FOLDER}/${CMAKE_STATIC_LIBRARY_PREFIX}PVRUtilsVk${CMAKE_STATIC_LIBRARY_SUFFIX}
${FRAMEWORK_LIB_FOLDER}/${CMAKE_STATIC_LIBRARY_PREFIX}PVRShell${CMAKE_STATIC_LIBRARY_SUFFIX}
${FRAMEWORK_LIB_FOLDER}/${CMAKE_STATIC_LIBRARY_PREFIX}PVRVk${CMAKE_STATIC_LIBRARY_SUFFIX}
${FRAMEWORK_LIB_FOLDER}/${CMAKE_STATIC_LIBRARY_PREFIX}PVRAssets${CMAKE_STATIC_LIBRARY_SUFFIX}
${FRAMEWORK_LIB_FOLDER}/${CMAKE_STATIC_LIBRARY_PREFIX}PVRCore${CMAKE_STATIC_LIBRARY_SUFFIX}
${EXTRA_LIBS})

target_compile_definitions(VulkanPostProcessing PUBLIC $<$<CONFIG:Debug>:DEBUG=1> $<$<NOT:$<CONFIG:Debug>>:RELEASE=1> ) #Defines DEBUG=1 or RELEASE=1

