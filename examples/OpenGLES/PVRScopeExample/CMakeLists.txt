cmake_minimum_required(VERSION 3.3)

project(OpenGLESPVRScopeExample)

set (OPENGLES_EXAMPLE 1)

#Include common functionality.  (Common.cmake)
# Sets up variables ( ${PROJECT_ARCH}, ${SDK_ROOT},${EXTERNAL_LIB_FOLDER}), sets up some defaults (e.g. CMAKE_BUILD_TYPE),sets up the include folders,
# sets up necessary libraries in EXTRA_LIBS (like dynamic linking, Android libaries, X11/xcb/Wayland etc. for Linux), 
# sets up some compilation flags (e.g. link time code generation, disables some warnings which hit on system files etc.)
include (../../../cmake/Common.cmake)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT OpenGLESPVRScopeExample)

set (SRC_FILES OpenGLESPVRScopeExample.cpp PVRScopeGraph.cpp PVRScopeGraph.h)

set (ASSET_FOLDER ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Assets_OpenGLESPVRScopeExample)

# ASSET FILES (textures, scene files, textures etc) - Packaged with the executable for runtime use
set (ASSET_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Satyr/Marble.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Satyr/Satyr.pod
	)
list (APPEND SRC_FILES ${ASSET_FILES})
source_group("assets" FILES ${ASSET_FILES})

# TEXTUAL SHADER FILES (OpenGL ES) - Packaged with the executable for runtime use
set (SHADER_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/FragShader_ES2.fsh
	${CMAKE_CURRENT_SOURCE_DIR}/FragShader_ES3.fsh
	${CMAKE_CURRENT_SOURCE_DIR}/VertShader_ES2.vsh
	${CMAKE_CURRENT_SOURCE_DIR}/VertShader_ES3.vsh
	${CMAKE_CURRENT_SOURCE_DIR}/GraphFragShader_ES2.fsh
	${CMAKE_CURRENT_SOURCE_DIR}/GraphFragShader_ES3.fsh
	${CMAKE_CURRENT_SOURCE_DIR}/GraphVertShader_ES2.vsh
	${CMAKE_CURRENT_SOURCE_DIR}/GraphVertShader_ES3.vsh
	)
list (APPEND SRC_FILES ${SHADER_FILES})
source_group("shaders" FILES ${SHADER_FILES})

set (RESOURCE_FILES ${ASSET_FILES} ${SHADER_FILES} ${SHADER_GENERATED_FILES})

# Adds Windows resouces.rc, macOS plists etc. For macOS/iOS, also set the opengl dynamic libs in the "frameworks" group
add_platform_specific_resource_files("SRC_FILES" "RESOURCE_FILES")

if (WIN32)
	add_executable( OpenGLESPVRScopeExample WIN32 ${SRC_FILES})
elseif (ANDROID)
	add_library( OpenGLESPVRScopeExample SHARED ${SRC_FILES})
elseif (APPLE)
	if (IOS)
		message("Skipping OpenGLESPVRScopeExample : Not supported on iOS")
		return()
	endif()
	add_executable(OpenGLESPVRScopeExample MACOSX_BUNDLE ${SRC_FILES} ${RESOURCE_FILES} ${FRAMEWORK_FILES})
	set_target_properties(OpenGLESPVRScopeExample PROPERTIES MACOSX_BUNDLE_INFO_PLIST  "${INFO_PLIST_FILE}")
	set_target_properties(OpenGLESPVRScopeExample PROPERTIES RESOURCE "${RESOURCE_FILES}")
elseif (UNIX OR QNX)
	add_executable( OpenGLESPVRScopeExample ${SRC_FILES})
endif()

add_rule_copy_assets_to_asset_folder("${RESOURCE_FILES}" "${ASSET_FOLDER}")

# Add the Framework subprojects.
if (NOT ANDROID) # Android builds the Framework libraries by using Gradle to kick CMake (this is an Android thing...)
	add_subdirectory_if_not_already_included(PVRCore ${SDK_ROOT}/framework/PVRCore ${FRAMEWORK_CMAKE_FILES_FOLDER}/PVRCore)
	add_subdirectory_if_not_already_included(PVRAssets ${SDK_ROOT}/framework/PVRAssets ${FRAMEWORK_CMAKE_FILES_FOLDER}/PVRAssets)
	add_subdirectory_if_not_already_included(PVRShell ${SDK_ROOT}/framework/PVRShell ${FRAMEWORK_CMAKE_FILES_FOLDER}/PVRShell)
	add_subdirectory_if_not_already_included(PVRUtilsGles ${SDK_ROOT}/framework/PVRUtils/OpenGLES ${FRAMEWORK_CMAKE_FILES_FOLDER}/PVRUtilsGles)

	add_dependencies(OpenGLESPVRScopeExample PVRCore  PVRAssets  PVRShell  PVRUtilsGles) #... We should still add the dependencies, build or not.
endif()

target_link_libraries(OpenGLESPVRScopeExample
${FRAMEWORK_LIB_FOLDER}/${CMAKE_STATIC_LIBRARY_PREFIX}PVRUtilsGles${CMAKE_STATIC_LIBRARY_SUFFIX}
${FRAMEWORK_LIB_FOLDER}/${CMAKE_STATIC_LIBRARY_PREFIX}PVRShell${CMAKE_STATIC_LIBRARY_SUFFIX}
${FRAMEWORK_LIB_FOLDER}/${CMAKE_STATIC_LIBRARY_PREFIX}PVRAssets${CMAKE_STATIC_LIBRARY_SUFFIX}
${FRAMEWORK_LIB_FOLDER}/${CMAKE_STATIC_LIBRARY_PREFIX}PVRCore${CMAKE_STATIC_LIBRARY_SUFFIX}
${EXTERNAL_LIB_FOLDER}/${CMAKE_STATIC_LIBRARY_PREFIX}PVRScopeDeveloper${CMAKE_STATIC_LIBRARY_SUFFIX}
${EXTRA_LIBS})

target_compile_definitions(OpenGLESPVRScopeExample PUBLIC $<$<CONFIG:Debug>:DEBUG=1> $<$<NOT:$<CONFIG:Debug>>:RELEASE=1> ) #Defines DEBUG=1 or RELEASE=1

if(WIN32)
	add_custom_command(TARGET OpenGLESPVRScopeExample POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${EXTERNAL_LIB_FOLDER}/${CMAKE_SHARED_LIBRARY_PREFIX}PVRScopeDeveloper${CMAKE_SHARED_LIBRARY_SUFFIX}" $<TARGET_FILE_DIR:OpenGLESPVRScopeExample>)
endif()

