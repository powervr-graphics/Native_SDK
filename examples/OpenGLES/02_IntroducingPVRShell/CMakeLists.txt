cmake_minimum_required(VERSION 3.3)

project(OpenGLESIntroducingPVRShell)

set (OPENGLES_EXAMPLE 1)

#Include common functionality.  (Common.cmake)
# Sets up variables ( ${PROJECT_ARCH}, ${SDK_ROOT},${EXTERNAL_LIB_FOLDER}), some defaults, e.g. CMAKE_BUILD_TYPE, selects the Windowing System,
# sets up the include folders, sets up some necessary libraries in EXTRA_LIBS, like the dynamic linking, android libaries, X11/xcb etc. for linux,
# sets up the compilers with for example link time code generation and disables/fixes some compiler flags we require
include (../../../cmake/Common.cmake) 

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT OpenGLESIntroducingPVRShell)

if(IOS)
	set (SRC_FILES EaglContext.mm  EglContext.h OpenGLESIntroducingPVRShell.cpp  )
else()
	set (SRC_FILES EglContext.cpp  EglContext.h  OpenGLESIntroducingPVRShell.cpp)
endif()

# Adds Windows resouces.rc, macOS plists etc. For MacOS/iOS, also set the opengl dynamic libs in the "frameworks" group
add_platform_specific_resource_files("SRC_FILES" "RESOURCE_FILES")

if (WIN32)
	add_executable( OpenGLESIntroducingPVRShell WIN32 ${SRC_FILES})
elseif (ANDROID)
	add_library( OpenGLESIntroducingPVRShell SHARED ${SRC_FILES})
elseif (APPLE)
	add_executable(OpenGLESIntroducingPVRShell MACOSX_BUNDLE ${SRC_FILES} ${ASSET_FILES} ${FRAMEWORK_FILES})
	set_target_properties(OpenGLESIntroducingPVRShell PROPERTIES MACOSX_BUNDLE_INFO_PLIST  "${INFO_PLIST_FILE}")
	set_target_properties(OpenGLESIntroducingPVRShell PROPERTIES RESOURCE "${ASSET_FILES}")
elseif (UNIX)
	add_executable( OpenGLESIntroducingPVRShell ${SRC_FILES})
endif()

add_rule_copy_assets_to_asset_folder("${RESOURCE_FILES}" "${ASSET_FOLDER}")

# Add the Framework subprojects.
if (NOT ANDROID) # Android builds the Framework libraries by using Gradle to kick CMake (this is an Android thing...)
	add_subdirectory_if_not_already_included(PVRCore ${SDK_ROOT}/framework/PVRCore ${FRAMEWORK_CMAKE_FILES_FOLDER}/PVRCore)
	add_subdirectory_if_not_already_included(PVRShell ${SDK_ROOT}/framework/PVRShell ${FRAMEWORK_CMAKE_FILES_FOLDER}/PVRShell)
endif()

target_link_libraries(OpenGLESIntroducingPVRShell PVRShell PVRCore ${EXTRA_LIBS})
target_compile_definitions(OpenGLESIntroducingPVRShell PUBLIC $<$<CONFIG:Debug>:DEBUG=1> $<$<NOT:$<CONFIG:Debug>>:RELEASE=1> ) #Defines DEBUG=1 or RELEASE=1
