cmake_minimum_required(VERSION 3.3)
project(OpenGLESPVRScopeExample)
if(APPLE)
	if(IOS)
		message("Skipping OpenGLESPVRScopeExample : Not supported on iOS")
		return()
	endif()
endif()
add_subdirectory(../../.. ${CMAKE_CURRENT_BINARY_DIR}/sdk)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT OpenGLESPVRScopeExample)

set(SRC_FILES OpenGLESPVRScopeExample.cpp PVRScopeGraph.cpp PVRScopeGraph.h)

set(ASSET_FOLDER ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Assets_OpenGLESPVRScopeExample)

# ASSET FILES (textures, scene files, textures etc) - Packaged with the executable for runtime use
set(ASSET_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Satyr/Marble.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Satyr/Satyr.pod
	)
list(APPEND SRC_FILES ${ASSET_FILES})
source_group("assets" FILES ${ASSET_FILES})

# TEXTUAL SHADER FILES (OpenGL ES) - Packaged with the executable for runtime use
set(SHADER_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/FragShader_ES2.fsh
	${CMAKE_CURRENT_SOURCE_DIR}/FragShader_ES3.fsh
	${CMAKE_CURRENT_SOURCE_DIR}/VertShader_ES2.vsh
	${CMAKE_CURRENT_SOURCE_DIR}/VertShader_ES3.vsh
	${CMAKE_CURRENT_SOURCE_DIR}/GraphFragShader_ES2.fsh
	${CMAKE_CURRENT_SOURCE_DIR}/GraphFragShader_ES3.fsh
	${CMAKE_CURRENT_SOURCE_DIR}/GraphVertShader_ES2.vsh
	${CMAKE_CURRENT_SOURCE_DIR}/GraphVertShader_ES3.vsh
	)
list(APPEND SRC_FILES ${SHADER_FILES})
source_group("shaders" FILES ${SHADER_FILES})

set(RESOURCE_FILES ${ASSET_FILES} ${SHADER_FILES})

# Adds Windows resouces.rc, macOS plists etc. For macOS/iOS, also set the opengl dynamic libs in the "frameworks" group
add_platform_specific_resource_files(SRC_FILES RESOURCE_FILES)

# Adds a platform specific executable
add_platform_specific_executable(OpenGLESPVRScopeExample "${SRC_FILES}" "${RESOURCE_FILES}")

# Copies the assets to our assets folder
add_rule_copy_assets_to_asset_folder("${RESOURCE_FILES}" "${ASSET_FOLDER}")

target_link_libraries(OpenGLESPVRScopeExample
	PVRShell
	PVRUtilsGles
	${SCOPE_LIB_FOLDER}/${CMAKE_STATIC_LIBRARY_PREFIX}PVRScopeDeveloper${CMAKE_STATIC_LIBRARY_SUFFIX}
)
if(WIN32)
	add_custom_command(TARGET OpenGLESPVRScopeExample POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${SCOPE_LIB_FOLDER}/${CMAKE_SHARED_LIBRARY_PREFIX}PVRScopeDeveloper${CMAKE_SHARED_LIBRARY_SUFFIX}" $<TARGET_FILE_DIR:OpenGLESPVRScopeExample>)
endif()