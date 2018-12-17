#THIS FILE:
# 1) Sets up variables:
#	${SDK_ROOT} 				: Root of the SDK
#	${FRAMEWORK_LIB_FOLDER} 	: Where the PowerVR Framework libraries are found
#	${EXTERNAL_LIB_FOLDER} 		: The prebuilts are found in (SDK_ROOT/lib...)
#	${PROJECT_ARCH}				: 32 or 64 (bits)
#	${SPIRV_COMPILER}			: SDK_ROOT/external/spir-v/glslangvalidator or glslangvalidator.exe
# 2) Sets up defaults:
#	${WS}						: The windowing system for Linux builds. Must normally be passed by the user (-DWS=X11 etc). Defaults to NullWS
#	${CMAKE_BUILD_TYPE}			: The build type. Normally Debug, Release, RelWithDebInfo, MinSizeRel. Defaults to Release
# 3) Sets up Include Directories (Framework and Builds/Include folders)
# 4) Links boilerplate libraries for Linux and Android:
#	Linux 						: libdl, libm
#	Android						: libandroid, liblog
# 5) Links Windowing system libraries for Linux
#	X11							: libx11, libxau
#	XCB							: libxcb
#	Wayland						: libWayland
# 6) Set the standard to C++14
set(CMAKE_CXX_STANDARD 14)

set(SDK_ROOT ${CMAKE_CURRENT_LIST_DIR}/..)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

include (${CMAKE_CURRENT_LIST_DIR}/Functions.cmake)

if(NOT CMAKE_BUILD_TYPE) #Default to release if the user passes nothing.
	message("-DCMAKE_BUILD_TYPE not defined. Assuming Release")
	set(CMAKE_BUILD_TYPE "Release")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/modules")  #for find_package

#Add common include folders
include_directories(${SDK_ROOT}/include/ ${SDK_ROOT}/framework/)

# Set SPIRV_COMPILER
if (WIN32)
	set (SPIRV_COMPILER "${SDK_ROOT}/external/spir-v/glslangValidator.exe")
else()
	set (SPIRV_COMPILER "${SDK_ROOT}/external/spir-v/glslangValidator")
endif()

# Add platform specific definitions (FRAMEWORK_LIB_FOLDER, WS_DEFINES, PROJECT_ARCH etc.)
include (${CMAKE_CURRENT_LIST_DIR}/PlatformSdkFolders.cmake)

# Populate EXTRA_LIBS with whatever platform specific libraries we need
include (${CMAKE_CURRENT_LIST_DIR}/PlatformLibraries.cmake)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${FRAMEWORK_LIB_FOLDER})

# For android, but it just doesn't hurt in any case
link_directories(${FRAMEWORK_LIB_FOLDER} ${EXTERNAL_LIB_FOLDER})

include(${CMAKE_CURRENT_LIST_DIR}/CompilerFlags.cmake)
