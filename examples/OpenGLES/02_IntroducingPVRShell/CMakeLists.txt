cmake_minimum_required(VERSION 3.3)
project(OpenGLESIntroducingPVRShell)

add_subdirectory(../../.. ${CMAKE_CURRENT_BINARY_DIR}/sdk)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT OpenGLESIntroducingPVRShell)

set(ASSET_FOLDER ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Assets_OpenGLESIntroducingPVRShell)

set(SRC_FILES EglContext.h OpenGLESIntroducingPVRShell.cpp)

if(IOS)
	list(APPEND SRC_FILES EaglContext.mm)
else()
	list(APPEND SRC_FILES EglContext.cpp)
endif()

# Adds Windows resouces.rc, macOS plists etc. For MacOS/iOS, also set the opengl dynamic libs in the "frameworks" group
add_platform_specific_resource_files(SRC_FILES RESOURCE_FILES)

add_platform_specific_executable(OpenGLESIntroducingPVRShell "${SRC_FILES}" "${RESOURCE_FILES}")

add_rule_copy_assets_to_asset_folder("${RESOURCE_FILES}" "${ASSET_FOLDER}")

target_link_libraries(OpenGLESIntroducingPVRShell 
	PVRShell
	OpenGLESPlatformInterface
)