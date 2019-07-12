cmake_minimum_required(VERSION 3.3)
project(VulkanNavigation3D)
if(IOS)
	message ("Skipping VulkanNavigation3D : Not supported on iOS")
	return()
endif()
add_subdirectory(../../.. ${CMAKE_CURRENT_BINARY_DIR}/sdk)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT VulkanNavigation3D)

set(SRC_FILES VulkanNavigation3D.cpp ../../common/NavDataProcess.cpp ../../common/NavDataProcess3D.cpp ../../common/NavDataProcess.h)

# ASSET FILES (textures, scene files, textures etc) - Packaged with the executable for runtime use
set(ASSET_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation3D/font.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation3D/Road.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation3D/map.osm
	)
list(APPEND SRC_FILES ${ASSET_FILES})
source_group("assets" FILES ${ASSET_FILES})

# SHADER SOURCE FILES (Vulkan) - Compiled to spir-v at build-time
set(SHADER_NAMES
AA_VertShader.vsh
AA_FragShader.fsh
VertShader.vsh
FragShader.fsh
PlanarShadow_VertShader.vsh
PlanarShadow_FragShader.fsh
PerVertexLight_VertShader.vsh
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
add_platform_specific_executable(VulkanNavigation3D "${SRC_FILES}" "${RESOURCE_FILES}")

# Copies the assets to our assets folder
if(APPLE)
	source_group(Resources FILES ${RESOURCE_FILES})
	set_source_files_properties(${RESOURCE_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
else()
	set(ASSET_FOLDER ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Assets_VulkanNavigation3D)
	add_rule_copy_assets_to_asset_folder("${RESOURCE_FILES}" "${ASSET_FOLDER}")
endif()

target_link_libraries(VulkanNavigation3D
	PVRShell
	PVRUtilsVk
)