cmake_minimum_required(VERSION 3.3)
project(VulkanHelloAPI)

if(IOS)
	message ("Skipping VulkanHelloAPI : Not supported on iOS")
	return()
endif()

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set (CMAKE_CXX_STANDARD 14)
set (SDK_ROOT ${CMAKE_CURRENT_LIST_DIR}/../../..)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${SDK_ROOT}/cmake/modules")  #for find_package

if (WIN32)
	#Get rid of the "this function is unsafe" warning in Visual Studio
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

if(NOT CMAKE_BUILD_TYPE) #Default to release if the user passes nothing.
	set(CMAKE_BUILD_TYPE "Release")
endif()

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT VulkanHelloAPI)

set(HelloAPI_INCLUDE_DIRECTORIES ${SDK_ROOT}/include/)

list(APPEND PLATFORM_LIBS ${CMAKE_DL_LIBS})

set(SRC_FILES vk_getProcAddrs.cpp VulkanHelloAPI.cpp vk_getProcAddrs.h VulkanHelloAPI.h VertShader.h FragShader.h VertShader.vsh FragShader.fsh)

if (WIN32)
	list (APPEND SRC_FILES 
		 MainWindows.cpp
		"${SDK_ROOT}/res/Windows/shared.rc"
		"${SDK_ROOT}/res/Windows/resource.h")
	add_executable(VulkanHelloAPI WIN32 ${SRC_FILES})
elseif (ANDROID)
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate") #This is to prevent the compiler for stripping out our "main" function
	find_library(lib-android android) #The following lines add Android specific libraries
	find_library(lib-log log)
	list(APPEND PLATFORM_LIBS ${lib-android} ${lib-log})
	list(APPEND SRC_FILES 
		MainAndroid.cpp 
		${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
	)
	list(APPEND HelloAPI_INCLUDE_DIRECTORIES ${ANDROID_NDK}/sources/android/native_app_glue)
	add_library(VulkanHelloAPI SHARED ${SRC_FILES})
	# Force export ANativeActivity_onCreate(),
	# Refer to: https://github.com/android-ndk/ndk/issues/381
	set_property(TARGET VulkanHelloAPI APPEND_STRING PROPERTY LINK_FLAGS " -u ANativeActivity_onCreate")
elseif (APPLE)
	set(PLATFORM_FOLDER macOS_x86)
	set(BUILDS_LIB_FOLDER "${SDK_ROOT}/lib/${PLATFORM_FOLDER}")
	list(APPEND SRC_FILES MainMacOS.mm ${SDK_ROOT}/framework/PVRShell/OS/AppKit/ViewMVK.mm)
	set(INFO_PLIST_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cmake-resources/macOS_Info.plist")
	find_library(lib-AppKit AppKit)
	list(APPEND PLATFORM_LIBS ${lib-AppKit})
	find_library(lib-QuartzCore QuartzCore)
	list(APPEND PLATFORM_LIBS ${lib-QuartzCore})
	find_library(lib-Metal Metal)
	list(APPEND PLATFORM_LIBS ${lib-Metal})
	list(APPEND ASSET_FILES "${CMAKE_CURRENT_SOURCE_DIR}/cmake-resources/MainMenu.xib")

	if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
		set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
		set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11")
		set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
	endif()

	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -all_load") # PREVENT THE LINKER FROM STRIPPING OUT THE FUNCTIONS THAT macOS CALLS REFLECTIVELY

	source_group(Resources FILES ${ASSET_FILES})
	set_source_files_properties(${ASSET_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

	if(EXISTS "${EXTERNAL_LIB_FOLDER}/libvulkan.dylib")
		list(APPEND FRAMEWORK_FILES "${EXTERNAL_LIB_FOLDER}/libvulkan.dylib")
	endif()

	set_source_files_properties(${FRAMEWORK_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Frameworks)
	source_group(Frameworks FILES ${FRAMEWORK_FILES})

	add_executable(VulkanHelloAPI MACOSX_BUNDLE ${SRC_FILES} ${ASSET_FILES} ${FRAMEWORK_FILES})
	set_target_properties(VulkanHelloAPI PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${INFO_PLIST_FILE}")
	set_target_properties(VulkanHelloAPI PROPERTIES RESOURCE "${ASSET_FILES}")
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
	
	if (${WS} STREQUAL X11) #The following lines add the specified windowing system libraries
		find_package(X11 REQUIRED)
		
		if(NOT ${X11_FOUND})
			message(FATAL_ERROR "X11 libraries could not be found. Please try setting: -DCMAKE_PREFIX_PATH pointing towards your X11 libraries")
		endif()
		
		list (APPEND PLATFORM_LIBS ${X11_LIBRARIES})
		include_directories(${X11_INCLUDE_DIR})
		
		list(APPEND SRC_FILES MainLinuxX11.cpp)
		add_definitions(-DBUILD_XLIB)
	elseif(${WS} STREQUAL Wayland)
		find_package(Wayland REQUIRED)
		find_library(lib-ffi ffi HINTS ${PKG_WAYLAND_LIBRARY_DIRS})
		
		if(NOT ${WAYLAND_FOUND})
			message("Wayland libraries could not be found. Please try setting: -DCMAKE_PREFIX_PATH pointing towards your Wayland libraries")
		endif()
		
		list (APPEND PLATFORM_LIBS ${WAYLAND_CLIENT_LIBRARIES} ${lib-ffi})
		include_directories(${WAYLAND_CLIENT_INCLUDE_DIR})
		
		list(APPEND SRC_FILES MainLinuxWayland.cpp)
		add_definitions (-DBUILD_WAYLAND)
	elseif(${WS} STREQUAL NullWS)
		list(APPEND SRC_FILES MainNullWS.cpp)
	else()
		message( FATAL_ERROR "Unrecognised WS: Valid values are NullWS(default), X11, Wayland." )
	endif()

	add_definitions(-D${WS}) #Add a compiler definition so that our header files know what we're building for
	add_executable(VulkanHelloAPI ${SRC_FILES})
endif()

if(PLATFORM_LIBS)
	target_link_libraries(VulkanHelloAPI ${PLATFORM_LIBS})
endif()
	
target_include_directories(VulkanHelloAPI PUBLIC ${HelloAPI_INCLUDE_DIRECTORIES})

target_compile_definitions(VulkanHelloAPI PUBLIC $<$<CONFIG:Debug>:DEBUG=1> $<$<NOT:$<CONFIG:Debug>>:RELEASE=1> ) #Defines DEBUG=1 or RELEASE=1