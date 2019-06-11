cmake_minimum_required(VERSION 3.3)
project(OpenGLESNavigation3D)
add_subdirectory(../../.. ${CMAKE_CURRENT_BINARY_DIR}/sdk)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT OpenGLESNavigation3D)

set(SRC_FILES OpenGLESNavigation3D.cpp ../../common/NavDataProcess3D.cpp ../../common/NavDataProcess.cpp ../../common/NavDataProcess.h)

set(ASSET_FOLDER ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Assets_OpenGLESNavigation3D)

# ASSET FILES (textures, scene files, textures etc) - Packaged with the executable for runtime use
set(ASSET_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation3D/font.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation3D/Road.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation3D/map.osm
	)
list(APPEND SRC_FILES ${ASSET_FILES})
source_group("assets" FILES ${ASSET_FILES})

# TEXTUAL SHADER FILES (OpenGL ES) - Packaged with the executable for runtime use
set(SHADER_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/AA_VertShader_ES2.vsh
	${CMAKE_CURRENT_SOURCE_DIR}/AA_FragShader_ES2.fsh
	${CMAKE_CURRENT_SOURCE_DIR}/VertShader_ES2.vsh
	${CMAKE_CURRENT_SOURCE_DIR}/FragShader_ES2.fsh
	${CMAKE_CURRENT_SOURCE_DIR}/PlanarShadow_VertShader_ES2.vsh
	${CMAKE_CURRENT_SOURCE_DIR}/PlanarShadow_FragShader_ES2.fsh
	${CMAKE_CURRENT_SOURCE_DIR}/PerVertexLight_VertShader_ES2.vsh
	)
list(APPEND SRC_FILES ${SHADER_FILES})
source_group("shaders" FILES ${SHADER_FILES})

set(RESOURCE_FILES ${ASSET_FILES} ${SHADER_FILES})

# Adds Windows resouces.rc, macOS plists etc. For macOS/iOS, also set the opengl dynamic libs in the "frameworks" group
add_platform_specific_resource_files(SRC_FILES RESOURCE_FILES)

# Adds a platform specific executable
add_platform_specific_executable(OpenGLESNavigation3D "${SRC_FILES}" "${RESOURCE_FILES}")

# Copies the assets to our assets folder
add_rule_copy_assets_to_asset_folder("${RESOURCE_FILES}" "${ASSET_FOLDER}")

target_link_libraries(OpenGLESNavigation3D
	PVRShell
	PVRUtilsGles
)