cmake_minimum_required(VERSION 3.3)
project(OpenGLESIntroducingPVRCamera)
add_subdirectory(../../.. ${CMAKE_CURRENT_BINARY_DIR}/sdk)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT OpenGLESIntroducingPVRCamera)

set(SRC_FILES OpenGLESIntroducingPVRCamera.cpp)

set(ASSET_FOLDER ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Assets_OpenGLESIntroducingPVRCamera)

# TEXTUAL SHADER FILES (OpenGL ES) - Packaged with the executable for runtime use
set(SHADER_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/VertShader.vsh
	${CMAKE_CURRENT_SOURCE_DIR}/FragShader.fsh
	)
list(APPEND SRC_FILES ${SHADER_FILES})
source_group("shaders" FILES ${SHADER_FILES})

set(RESOURCE_FILES ${ASSET_FILES} ${SHADER_FILES})

# Adds Windows resouces.rc, macOS plists etc. For macOS/iOS, also set the opengl dynamic libs in the "frameworks" group
add_platform_specific_resource_files(SRC_FILES RESOURCE_FILES)

# Adds a platform specific executable
add_platform_specific_executable(OpenGLESIntroducingPVRCamera "${SRC_FILES}" "${RESOURCE_FILES}")

# Copies the assets to our assets folder
add_rule_copy_assets_to_asset_folder("${RESOURCE_FILES}" "${ASSET_FOLDER}")

target_link_libraries(OpenGLESIntroducingPVRCamera
	PVRShell
	PVRUtilsGles
	PVRCamera
)