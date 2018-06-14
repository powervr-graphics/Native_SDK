#THIS FILE:
# 1) Sets up variables:
#   ${SDK_ROOT} 				: Root of the SDK
#   ${FRAMEWORK_LIB_FOLDER} 	: Where the PowerVR Framework libraries are found
#   ${BUILDS_LIB_FOLDER} 		: The prebuilts are found in (SDK_ROOT/lib...)
#	${PROJECT_ARCH}				: 32 or 64 (bits)
# 2) Sets up defaults:
#	${WS}						: The windowing system for Linux builds. Must normally be passed by the user (-DWS=X11 etc). Defaults to NullWS
#	${CMAKE_BUILD_TYPE}			: The build type. Normally Debug, Release, RelWithDebInfo, MinSizeRel. Defaults to Release
# 3) Sets up Include Directories (Framework and Builds/Include folders)
# 4) Links boilerplate libraries for Linux and Android:
# 	Linux 						: libdl, libm
#	Android						: libandroid, liblog
# 5) Links Windowing system libraries for Linux
#	X11							: libx11, libxau
#	XCB							: libxcb
#	Wayland						: libWayland
# 6) Set the standard to C++14
set(CMAKE_CXX_STANDARD 14)
set(SDK_ROOT ${CMAKE_CURRENT_LIST_DIR}/..)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

if(NOT CMAKE_BUILD_TYPE) #Default to release if the user passes nothing.
	message("-DCMAKE_BUILD_TYPE not defined. Assuming Release")
	set(CMAKE_BUILD_TYPE "Release")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/modules")  #for find_package

#Add common include folders
include_directories(
	${SDK_ROOT}/framework/
	${SDK_ROOT}/include/
)

# Set PROJECT_ARCH to either 32 or 64 (bits)
if ("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
	set (PROJECT_ARCH 64)
else()
	set (PROJECT_ARCH 32)
endif()

# Add platform specific definitions, link platform specific libraries, link windowing system libraries, add android stuff.
if (WIN32)
#For Windows, the library paths are dependent on configuration(Debug/Release), so must depend on $<CONFIG>.
#But the Cmake files folder cannot be dependent on $<CONFIG> as the same project builds both debug and release
	set(FRAMEWORK_LIB_FOLDER "${SDK_ROOT}/framework/bin/Windows_x86_${PROJECT_ARCH}/$<CONFIG>")
	set(FRAMEWORK_CMAKE_FILES_FOLDER "${SDK_ROOT}/framework/bin/Windows_x86_${PROJECT_ARCH}/cmake")
	set(BUILDS_LIB_FOLDER "${SDK_ROOT}/lib/Windows_x86_${PROJECT_ARCH}")
elseif (ANDROID)
	find_library( lib-android android) #The following lines add Android specific libraries
	find_library( lib-log log) 
	find_library( lib-dl dl) 
	set(EXTRA_LIBS ${lib-android} ${lib-log} ${lib-dl} )
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate") #This is to prevent the compiler for stripping out our "main" function
	set(FRAMEWORK_LIB_FOLDER "${SDK_ROOT}/framework/bin/Android/${CMAKE_BUILD_TYPE}/${ANDROID_ABI}")
	set(FRAMEWORK_CMAKE_FILES_FOLDER "${SDK_ROOT}/framework/bin/Android/${CMAKE_BUILD_TYPE}/${ANDROID_ABI}/cmake")
	set(BUILDS_LIB_FOLDER "${SDK_ROOT}/lib/Android/${ANDROID_ABI}")
elseif (APPLE)

	if (IOS)
		set (PLATFORM_FOLDER iOS)
		find_library(lib-uikit UIKit)
		find_library(lib-gles OpenGLES)
		find_library(lib-foundation Foundation)
		find_library(lib-avfoundation AVFoundation)
		find_library(lib-quartz QuartzCore)
		find_library(lib-coremedia CoreMedia)
		find_library(lib-corevideo CoreVideo)
		set(libs ${lib-uikit} ${lib-gles} ${lib-foundation} ${lib-avfoundation} ${lib-quartz} ${lib-coremedia} ${lib-corevideo})
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-return-type" CACHE STRING "")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-return-type" CACHE STRING "")
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wno-return-type" CACHE STRING "")
	else()
		set (PLATFORM_FOLDER macOS_x86)
		find_library(lib-appkit AppKit)
		set(libs ${lib-appkit})
		if(DEFINED OPENGLES_EXAMPLE)
			find_library(lib-opencl OpenCL)
			list (APPEND libs ${lib-opencl})
		endif()
		if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
			set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
			set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11")
			set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
		endif()
	endif()
	set (EXTRA_LIBS ${libs})
	set(FRAMEWORK_LIB_FOLDER "${SDK_ROOT}/framework/bin/${PLATFORM_FOLDER}/$<CONFIG>")
	set(FRAMEWORK_CMAKE_FILES_FOLDER "${SDK_ROOT}/framework/bin/${PLATFORM_FOLDER}/cmake")
	set(BUILDS_LIB_FOLDER "${SDK_ROOT}/lib/${PLATFORM_FOLDER}")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -all_load")
	 #PREVENT THE LINKER FROM STRIPPING OUT THE FUNCTIONS THAT macOS CALLS REFLECTIVELY
elseif (UNIX)
	set(WS_DEFINE "")
	if(NOT WS) #We support building for several Windowing Systems. Typical desktop systems support X11 and Wayland is catching on. NullWS is used by some development platforms/ testchip.
		message ("WS Variable not set. Assuming NullWS. If you wish to build for X11, Wayland or another supported windowing system, please pass -DWS=X11, -DWS=Wayland etc. to CMake")
		set(WS "NullWS")
		set(WS_DEFINE "${WS}")
	endif()
	
	if(NOT DEFINED CMAKE_PREFIX_PATH)
		set(CMAKE_PREFIX_PATH $ENV{CMAKE_PREFIX_PATH})
	endif()
	
	set(EXTRA_LIBS ${CMAKE_DL_LIBS})

	if (${WS} STREQUAL X11) #The following lines add the specified windowing system libraries
		find_package(X11 REQUIRED)
		
		if(NOT ${X11_FOUND})
			message("X11 libraries could not be found. Please try setting: -DCMAKE_PREFIX_PATH pointing towards your X11 libraries")
		endif()
		
		list (APPEND EXTRA_LIBS ${X11_LIBRARIES})
		include_directories(${X11_INCLUDE_DIR})
		set(WS_DEFINE "${WS}")

                if(X11_FOUND)
                   add_definitions(-DVK_USE_PLATFORM_XLIB_KHR)
                endif()
                find_package(X11_XCB)
                if(X11_XCB_FOUND)
                    add_definitions(-DVK_USE_PLATFORM_XCB_KHR)
                endif()
	elseif(${WS} STREQUAL XCB)
		# XCB isn't currently being used by the PowerVR SDK but the following would allow you to link against the XCB libraries
		find_package(XCB REQUIRED)
		
		if(NOT ${XCB_FOUND})
			message("XCB libraries could not be found. Please try setting: -DCMAKE_PREFIX_PATH pointing towards your XCB libraries")
		endif()
		
		list (APPEND EXTRA_LIBS ${XCB_LIBRARIES})
		include_directories(${XCB_INCLUDE_DIRS})
		set(WS_DEFINE "${WS}")
	elseif(${WS} STREQUAL Wayland)
		find_package(Wayland REQUIRED)
		if(DEFINED OPENGLES_EXAMPLE)
			list (APPEND EXTRA_LIBS ${WAYLAND_EGL_LIBRARIES})
			include_directories(${WAYLAND_EGL_INCLUDE_DIR})
		endif()
		
		if(NOT ${WAYLAND_FOUND})
			message("Wayland libraries could not be found. Please try setting: -DCMAKE_PREFIX_PATH pointing towards your Wayland libraries")
		endif()
		
		find_library(lib-ffi ffi HINTS ${PKG_WAYLAND_LIBRARY_DIRS})
		list (APPEND EXTRA_LIBS ${WAYLAND_CLIENT_LIBRARIES} ${lib-ffi})
		include_directories(${WAYLAND_CLIENT_INCLUDE_DIR})
                add_definitions(-DVK_USE_PLATFORM_WAYLAND_KHR)
		set(WS_DEFINE "WAYLAND")
	elseif(${WS} STREQUAL NullWS)
		set(WS_DEFINE "${WS}")
	elseif(${WS} STREQUAL Screen)
		if(CMAKE_SYSTEM_NAME MATCHES "QNX")
			list (APPEND EXTRA_LIBS "screen")
		endif()
		set(WS_DEFINE "${WS}")
	else()
		message( FATAL_ERROR "Unrecognised WS: Valid values are NullWS(default), X11, Wayland, Screen." )
	endif()

	add_definitions (-D${WS_DEFINE}) #Add a compiler definition so that our header files know what we're building for
	
	if(CMAKE_SYSTEM_NAME MATCHES "QNX")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-ignored-attributes")
	else()
		find_package(Threads)
		list (APPEND EXTRA_LIBS ${CMAKE_THREAD_LIBS_INIT})
		list (APPEND EXTRA_LIBS "rt")
	endif()
	
	set(FRAMEWORK_LIB_FOLDER "${SDK_ROOT}/framework/bin/${CMAKE_SYSTEM_NAME}_${CMAKE_SYSTEM_PROCESSOR}/${CMAKE_BUILD_TYPE}${WS}")
	set(FRAMEWORK_CMAKE_FILES_FOLDER "${SDK_ROOT}/framework/bin/${CMAKE_SYSTEM_NAME}_${CMAKE_SYSTEM_PROCESSOR}/${CMAKE_BUILD_TYPE}${WS}/cmake")
	set(BUILDS_LIB_FOLDER "${SDK_ROOT}/lib/${CMAKE_SYSTEM_NAME}_${CMAKE_SYSTEM_PROCESSOR}")
else()
	message(FATAL_ERROR "UNKNOWN PLATFORM - Please set up this line with a path to where the Framework libraries must be set")
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${FRAMEWORK_LIB_FOLDER})

#some optimizations for windows
if (WIN32)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	#Enable Link Time Code Generation/Whole program optimizations
	set (CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG")
	set (CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} /LTCG")
	set (CMAKE_EXE_LINKER_FLAGS_MINSIZEREL "${CMAKE_EXE_LINKER_FLAGS_MINSIZEREL} /LTCG")
	set (CMAKE_STATIC_LINKER_FLAGS_RELEASE "${CMAKE_STATIC_LINKER_FLAGS_RELEASE} /LTCG")
	set (CMAKE_STATIC_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_STATIC_LINKER_FLAGS_RELWITHDEBINFO} /LTCG")
	set (CMAKE_STATIC_LINKER_FLAGS_MINSIZEREL "${CMAKE_STATIC_LINKER_FLAGS_MINSIZEREL} /LTCG")
	add_compile_options($<$<NOT:$<CONFIG:DEBUG>>:/GL>)
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations -Wno-reorder")
	include(CheckCXXCompilerFlag)
	CHECK_CXX_COMPILER_FLAG(-fno-strict-aliasing COMPILER_SUPPORTS_NO_STRICT_ALIASING)
	if(COMPILER_SUPPORTS_NO_STRICT_ALIASING)
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fno-strict-aliasing")
	endif()
	if(CMAKE_COMPILER_IS_GNUCXX AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.9) # workaround an issue observed when using the optimation flag "-ftree-slp-vectorize" which is enabled when using the optimisation level "-O3" with gcc versions < 4.9.
		CHECK_CXX_COMPILER_FLAG(-fno-tree-slp-vectorize COMPILER_SUPPORTS_TREE_SLP_VECTORIZE)
		if(COMPILER_SUPPORTS_TREE_SLP_VECTORIZE)
			set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fno-tree-slp-vectorize")
		endif()
	endif()
	CHECK_CXX_COMPILER_FLAG(-ffast-math COMPILER_SUPPORTS_FAST_MATH)
	if(COMPILER_SUPPORTS_FAST_MATH)
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -ffast-math")
	endif()
endif()

