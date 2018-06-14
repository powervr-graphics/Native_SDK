cmake_minimum_required(VERSION 3.3)

project(OpenGLESIntroducingPVRShell)

set (OPENGLES_EXAMPLE 1)

#Include common functionality.  (Common.cmake)
#Common.cmake performs the following operations
# 1) Sets up variables 
#	${PROJECT_ARCH}				: 32 or 64 (bits)
#   ${SDK_ROOT} 				: Root of the SDK
#   ${FRAMEWORK_LIB_FOLDER} 	: Where the PowerVR Framework libraries are found
#   ${BUILDS_LIB_FOLDER} 		: Where the prebuilts are found (SDK_ROOT/lib/...)
# 2) Sets up defaults:
#	${CMAKE_BUILD_TYPE}			: The build type. Normally Debug, Release, RelWithDebInfo, MinSizeRel. Defaults to Release
#	${WS}						: The windowing system for Linux builds. Must normally be passed by the user (-DWS=X11 etc). Defaults to NullWS
# 3) Sets up Include Directories (Framework and Builds/Include folders)
# 4) Sets ${EXTRA_LIBS} to any necessary boilerplate libraries for Linux and Android
# 	Linux 						: libdl, libm, libpthread...
#	Android						: libandroid, liblog
# 5) Links Windowing system libraries for Linux (depending on the WS variable)
#	X11							: libx11, libxau
#	XCB							: libxcb
#	Wayland						: libWayland
# 6) Set the standard to C++14
include (../../../cmake/Common.cmake) 

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT OpenGLESIntroducingPVRShell)

if(IOS)
	set (SRC_FILES EaglContext.mm  EglContext.h OpenGLESIntroducingPVRShell.cpp  )
else()
	set (SRC_FILES EglContext.cpp  EglContext.h  OpenGLESIntroducingPVRShell.cpp)
endif()

if (WIN32)
	list (APPEND SRC_FILES  #Add the resource files needed for windows (icons, asset files etc).
		"${SDK_ROOT}/res/Windows/shared.rc"
		"${SDK_ROOT}/res/Windows/resource.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/cmake-resources/Resources.rc")
	add_executable( OpenGLESIntroducingPVRShell WIN32 ${SRC_FILES})
elseif (ANDROID)
	add_library( OpenGLESIntroducingPVRShell SHARED ${SRC_FILES})
elseif (APPLE)
	if (IOS)
		set(INFO_PLIST_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cmake-resources/iOS_Info.plist")
		file(GLOB ICONS LIST_DIRECTORIES false ${SDK_ROOT}/res/iOS/* ${SDK_ROOT}/res/iOS/OpenGLES/*)
		list(APPEND ASSET_FILES ${ICONS})
	else()
		set(INFO_PLIST_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cmake-resources/macOS_Info.plist")
		list(APPEND FRAMEWORK_FILES "${BUILDS_LIB_FOLDER}/libEGL.dylib" "${BUILDS_LIB_FOLDER}/libGLESv2.dylib")
		set_source_files_properties(${FRAMEWORK_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Frameworks)
		source_group(Frameworks FILES ${FRAMEWORK_FILES})
		list(APPEND ASSET_FILES "${CMAKE_CURRENT_SOURCE_DIR}/cmake-resources/MainMenu.xib")
	endif()
	source_group(Resources FILES ${ASSET_FILES})
	set_source_files_properties(${ASSET_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
	add_executable(OpenGLESIntroducingPVRShell MACOSX_BUNDLE ${SRC_FILES} ${ASSET_FILES} ${FRAMEWORK_FILES})
	set_target_properties(OpenGLESIntroducingPVRShell PROPERTIES MACOSX_BUNDLE_INFO_PLIST  "${INFO_PLIST_FILE}")
	set_target_properties(OpenGLESIntroducingPVRShell PROPERTIES RESOURCE "${ASSET_FILES}")
elseif (UNIX)
#Copy all assets to the Assets folder in order for the executable to be able to locate it
	foreach (ASSET_FILE_PATH ${ASSET_FILES})
		get_filename_component(ASSET_FILE_NAME ${ASSET_FILE_PATH} NAME)
		configure_file(${ASSET_FILE_PATH} Assets_OpenGLESIntroducingPVRShell/${ASSET_FILE_NAME} COPYONLY)
	endforeach()
	add_executable( OpenGLESIntroducingPVRShell ${SRC_FILES})
endif()

# Add the Framework subprojects.
if (NOT ANDROID) # Android builds the Framework libraries by using Gradle to kick CMake (this is an Android thing...)
	if (NOT TOP_LEVEL_BUILD) # If we are using the top level CMakeLists.txt (found in the SDK root), we ensure that we build the Framework
	#and all the Examples once. If TOP_LEVEL_BUILD is not defined, it means that we are building each example one-by-one, so we
	#need to build the Framework. 
		add_subdirectory(${SDK_ROOT}/framework/PVRCore ${FRAMEWORK_CMAKE_FILES_FOLDER}/PVRCore)
		add_subdirectory(${SDK_ROOT}/framework/PVRShell ${FRAMEWORK_CMAKE_FILES_FOLDER}/PVRShell)
endif()
	add_dependencies(OpenGLESIntroducingPVRShell PVRCore  PVRShell) #... We should still add the dependencies, build or not.
endif()

target_link_libraries(	OpenGLESIntroducingPVRShell
						${FRAMEWORK_LIB_FOLDER}/${CMAKE_STATIC_LIBRARY_PREFIX}PVRShell${CMAKE_STATIC_LIBRARY_SUFFIX}
						${FRAMEWORK_LIB_FOLDER}/${CMAKE_STATIC_LIBRARY_PREFIX}PVRCore${CMAKE_STATIC_LIBRARY_SUFFIX}
						${EXTRA_LIBS}
					)
target_compile_definitions( OpenGLESIntroducingPVRShell PUBLIC $<$<CONFIG:Debug>:DEBUG=1> $<$<NOT:$<CONFIG:Debug>>:RELEASE=1> ) #Defines DEBUG=1 or RELEASE=1
