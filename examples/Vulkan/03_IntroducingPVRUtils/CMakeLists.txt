cmake_minimum_required(VERSION 3.3)

project(VulkanIntroducingPVRUtils)

set (VULKAN_EXAMPLE 1)

#Include common functionality.  (Common.cmake)
#Common.cmake performs the following operations
# 1) Sets up variables 
#	${PROJECT_ARCH}				: 32 or 64 (bits)
#   ${SDK_ROOT} 				: Root of the SDK
#   ${FRAMEWORK_LIB_FOLDER} 	: Where the PowerVR Framework libraries are found
#   ${BUILDS_LIB_FOLDER} 		: The prebuilts are found in (SDK_ROOT/lib....)
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

if(APPLE)
	message ("Vulkan is not supported on Apple platforms.")
	return()
endif()

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT VulkanIntroducingPVRUtils)

set (SRC_FILES VulkanIntroducingPVRUtils.cpp  )

set (ASSET_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/GnomeToy/GnomeWood.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/GnomeToy/DarkWood.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/GnomeToy/ClearWood.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/GnomeToy/GnomeToy.pod
	${CMAKE_CURRENT_SOURCE_DIR}/VertShader_vk.spv
	${CMAKE_CURRENT_SOURCE_DIR}/FragShader_vk.spv
	${CMAKE_CURRENT_SOURCE_DIR}/VertShader_vk.vsh
	${CMAKE_CURRENT_SOURCE_DIR}/FragShader_vk.fsh
	${CMAKE_CURRENT_SOURCE_DIR}/recompile.sh
	${CMAKE_CURRENT_SOURCE_DIR}/recompile.bat
	)

list (APPEND SRC_FILES ${ASSET_FILES})
source_group("assets" FILES ${ASSET_FILES})


if (WIN32)
	list (APPEND SRC_FILES  #Add the resource files needed for windows (icons, asset files etc).
		"${SDK_ROOT}/res/Windows/shared.rc"
		"${SDK_ROOT}/res/Windows/resource.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/cmake-resources/Resources.rc")
	add_executable( VulkanIntroducingPVRUtils WIN32 ${SRC_FILES})
elseif (ANDROID)
	add_library( VulkanIntroducingPVRUtils SHARED ${SRC_FILES})

elseif (UNIX)
add_executable( VulkanIntroducingPVRUtils ${SRC_FILES})
#Copy all assets to the Assets folder in order for the executable to be able to locate it
get_target_property(BIN_PATH VulkanIntroducingPVRUtils  RUNTIME_OUTPUT_DIRECTORY)
foreach (ASSET_FILE_PATH ${ASSET_FILES})
	get_filename_component(ASSET_FILE_NAME ${ASSET_FILE_PATH} NAME)
	configure_file(${ASSET_FILE_PATH} ${BIN_PATH}/Assets_VulkanIntroducingPVRUtils/${ASSET_FILE_NAME} COPYONLY)
endforeach()
endif()

# Add the Framework subprojects.
if (NOT ANDROID) # Android builds the Framework libraries by using Gradle to kick CMake (this is an Android thing...)
	if (NOT TOP_LEVEL_BUILD) # If we are using the top level CMakeLists.txt (found in the SDK root), we ensure that we build the Framework
	#and all the Examples once. If TOP_LEVEL_BUILD is not defined, it means that we are building each example one-by-one, so we
	#need to build the Framework. 
		add_subdirectory(${SDK_ROOT}/framework/PVRCore ${FRAMEWORK_CMAKE_FILES_FOLDER}/PVRCore)
		add_subdirectory(${SDK_ROOT}/framework/PVRShell ${FRAMEWORK_CMAKE_FILES_FOLDER}/PVRShell)
		add_subdirectory(${SDK_ROOT}/framework/PVRAssets ${FRAMEWORK_CMAKE_FILES_FOLDER}/PVRAssets)
		add_subdirectory(${SDK_ROOT}/framework/PVRVk ${FRAMEWORK_CMAKE_FILES_FOLDER}/PVRVk)
		add_subdirectory(${SDK_ROOT}/framework/PVRUtils/Vulkan ${FRAMEWORK_CMAKE_FILES_FOLDER}/PVRUtilsVk)
endif()
	add_dependencies(VulkanIntroducingPVRUtils PVRCore  PVRShell  PVRAssets  PVRVk  PVRUtilsVk) #... We should still add the dependencies, build or not.
endif()

target_link_libraries(	VulkanIntroducingPVRUtils
						${FRAMEWORK_LIB_FOLDER}/${CMAKE_STATIC_LIBRARY_PREFIX}PVRUtilsVk${CMAKE_STATIC_LIBRARY_SUFFIX}
						${FRAMEWORK_LIB_FOLDER}/${CMAKE_STATIC_LIBRARY_PREFIX}PVRVk${CMAKE_STATIC_LIBRARY_SUFFIX}
						${FRAMEWORK_LIB_FOLDER}/${CMAKE_STATIC_LIBRARY_PREFIX}PVRAssets${CMAKE_STATIC_LIBRARY_SUFFIX}
						${FRAMEWORK_LIB_FOLDER}/${CMAKE_STATIC_LIBRARY_PREFIX}PVRShell${CMAKE_STATIC_LIBRARY_SUFFIX}
						${FRAMEWORK_LIB_FOLDER}/${CMAKE_STATIC_LIBRARY_PREFIX}PVRCore${CMAKE_STATIC_LIBRARY_SUFFIX}
						${EXTRA_LIBS}
					)
target_compile_definitions( VulkanIntroducingPVRUtils PUBLIC $<$<CONFIG:Debug>:DEBUG=1> $<$<NOT:$<CONFIG:Debug>>:RELEASE=1> ) #Defines DEBUG=1 or RELEASE=1
