cmake_minimum_required(VERSION 3.18.0 FATAL_ERROR)
project(PVRShell)
include(GNUInstallDirs)
include(../FrameworkCommon.cmake)
# Copyright (c) Imagination Technologies Limited.

# Prevent PVRShell being added multiple times
if(TARGET PVRShell)
	return()
endif()

# Add PVRCore
add_subdirectory(../PVRCore PVRCore)

message(STATUS "Adding PVRShell")

# Set the common source list for PVRShell
set(PVRShell_HEADERS
	../../include/sdkver.h
	PVRShell.h
	Shell.h
	ShellData.h
	StateMachine.h
	OS/ShellOS.h)
	
set(PVRShell_SRC
	Shell.cpp
	StateMachine.cpp)

# Each platform links against different libraries
# So we have to initialise an empty list of libraries to track
set(PVRShell_PRIVATE_LINK_LIBS "")

# Each platform has its own implementation of PVRShell
# So we have to append a different list of headers, sources
# and link libraries for each platform
if(WIN32)
	list(APPEND PVRShell_HEADERS
		OS/Windows/WindowsOSData.h)
	list(APPEND PVRShell_SRC
		OS/Windows/ShellOS.cpp
		EntryPoint/WinMain/main.cpp)
elseif(ANDROID)
	find_library(lib-android android)
	list(APPEND PVRShell_PRIVATE_LINK_LIBS ${lib-android})
	list(APPEND PVRShell_SRC
		EntryPoint/android_main/main.cpp
		OS/Android/ShellOS.cpp
		${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)
	
	list(APPEND PVRShell_PRIVATE_INCLUDE_DIRECTORIES ${ANDROID_NDK}/sources/android/native_app_glue)
elseif(UNIX)
	list(APPEND PVRShell_OS_HEADERS
		OS/Linux/InternalOS.h)
	list(APPEND PVRShell_SRC
		OS/Linux/InternalOS.cpp)

	# We support building for several Windowing Systems. 
	# Typical desktop systems support X11 and Wayland is catching on. 
	# NullWS is used by some development platforms/testchip.
	if(PVR_WINDOW_SYSTEM)
		
		set(PVR_WINDOW_SYSTEM_DEFINE "" CACHE INTERNAL "")
		
		# Validate that the user has passed a supported value into-DPVR_WINDOW_SYSTEM
		if(${PVR_WINDOW_SYSTEM} STREQUAL X11 
			OR ${PVR_WINDOW_SYSTEM} STREQUAL XCB 
			OR ${PVR_WINDOW_SYSTEM} STREQUAL Wayland 
			OR ${PVR_WINDOW_SYSTEM} STREQUAL NullWS 
			OR ${PVR_WINDOW_SYSTEM} STREQUAL Screen)
			set(PVR_WINDOW_SYSTEM_DEFINE "${PVR_WINDOW_SYSTEM}" CACHE INTERNAL "Windowing system define for Linux")
		else()
			message(FATAL_ERROR "Unrecognised PVR_WINDOW_SYSTEM: Valid values are NullWS, X11, XCB, Wayland, Screen.")
		endif()

		# X11 OR XCB is to be used
		if(${PVR_WINDOW_SYSTEM} STREQUAL X11 OR ${PVR_WINDOW_SYSTEM} STREQUAL XCB)
			find_package(X11 REQUIRED)

			if(${X11_FOUND})
				list(APPEND PVRShell_PRIVATE_LINK_LIBS ${X11_LIBRARIES})
				list(APPEND PVRShell_PRIVATE_INCLUDE_DIRECTORIES ${X11_INCLUDE_DIR})
			else()
				message(FATAL_ERROR "X11 libraries could not be found. Please try setting: -DCMAKE_PREFIX_PATH pointing towards your X11 libraries")
			endif()
			
			# XCB is to be used
			if(${PVR_WINDOW_SYSTEM} STREQUAL XCB)
				find_package(XCB REQUIRED)
				
				if(${XCB_FOUND})
					# Xlib-xcb is also used for XCB specifically for OpenGLES
					find_package(X11_XCB)
				
					if(NOT ${X11_XCB_FOUND})
						message(FATAL_ERROR "X11_XCB libraries could not be found. Please try setting: -DCMAKE_PREFIX_PATH pointing towards your XCB libraries")
					endif()

					# Add the XCB and Xlib-xcb libraries and include directories
					list(APPEND PVRShell_PRIVATE_LINK_LIBS ${XCB_LIBRARIES} ${X11_XCB_LIBRARIES})
					list(APPEND PVRShell_PRIVATE_INCLUDE_DIRECTORIES ${XCB_INCLUDE_DIRS})
				else()
					message(FATAL_ERROR "XCB libraries could not be found. Please try setting: -DCMAKE_PREFIX_PATH pointing towards your XCB libraries")
				endif()
			endif()
		# The user has requested the Wayland libraries
		elseif(${PVR_WINDOW_SYSTEM} STREQUAL Wayland)
			find_package(Wayland REQUIRED)
			
			if(NOT ${WAYLAND_FOUND})
				message(FATAL_ERROR "Wayland libraries could not be found. Please try setting: -DCMAKE_PREFIX_PATH pointing towards your Wayland libraries")
			endif()
			
			find_library(lib-ffi ffi HINTS ${PKG_WAYLAND_LIBRARY_DIRS})
			
			list(APPEND PVRShell_PRIVATE_LINK_LIBS ${WAYLAND_CLIENT_LIBRARIES} ${lib-ffi})
			list(APPEND PVRShell_PRIVATE_INCLUDE_DIRECTORIES ${WAYLAND_CLIENT_INCLUDE_DIR})
		# The user has requested no windowing system(direct-to-framebuffer)
		elseif(${PVR_WINDOW_SYSTEM} STREQUAL NullWS) 
		else()
			message(FATAL_ERROR "Unrecognised PVR_WINDOW_SYSTEM: Valid values are NullWS(default), X11, XCB, Wayland." )
		endif()

		# Now that the windowing system has been validated
		# Add the PVRShell platform specific sources 
		if("${PVR_WINDOW_SYSTEM}" STREQUAL "NullWS")
			message("CMake: Generating PVRShell for NullWS")
			list(APPEND PVRShell_SRC
				OS/Linux/NullWS/ShellOS.cpp
				EntryPoint/main/main.cpp)
		elseif("${PVR_WINDOW_SYSTEM}" STREQUAL "X11")
			message("CMake: Generating PVRShell for X11")
			list(APPEND PVRShell_SRC
				OS/Linux/X11/ShellOS.cpp
				EntryPoint/main/main.cpp)
		elseif("${PVR_WINDOW_SYSTEM}" STREQUAL "XCB")
			message("CMake: Generating PVRShell for XCB")
			list(APPEND PVRShell_SRC
				OS/Linux/XCB/ShellOS.cpp
				EntryPoint/main/main.cpp)
		elseif("${PVR_WINDOW_SYSTEM}" STREQUAL "Wayland")
			message("CMake: Generating PVRShell for Wayland")
			list(APPEND PVRShell_SRC
				OS/Linux/Wayland/ShellOS.cpp
				EntryPoint/main/main.cpp)
		endif()	
	else()
		message(FATAL_ERROR "PVR_WINDOW_SYSTEM (Window System) Variable has been not set for PVRShell. Supported windowing systems can be enabled by passing: -DPVR_WINDOW_SYSTEM=NullWS, -DPVR_WINDOW_SYSTEM=X11, -DPVR_WINDOW_SYSTEM=XCB, -DPVR_WINDOW_SYSTEM=Wayland, -DPVR_WINDOW_SYSTEM=Screen to CMake")
	endif()
endif()

# Create the library
add_library(PVRShell STATIC ${PVRShell_SRC} ${PVRShell_HEADERS})

# Link PVRShell to PVRCore and it's windowing specific libraries
target_link_libraries(PVRShell PUBLIC PVRCore PRIVATE ${PVRShell_PRIVATE_LINK_LIBS})

# Allow PVRShell to see the rest of the framework 
# And the windowing system specific include directories
target_include_directories(PVRShell 
	PUBLIC 
		${CMAKE_CURRENT_LIST_DIR}/..
		${CMAKE_CURRENT_LIST_DIR}/../../include
	PRIVATE
		${PVRShell_PRIVATE_INCLUDE_DIRECTORIES})

# Add the windowing system define variable, so that the source can know which Windowing was requsted
target_compile_definitions(PVRShell PUBLIC ${PVR_WINDOW_SYSTEM_DEFINE})

# Apply the common framework compile definitions to PVRShell
# This sets certain warnings on/off and adds appropriate
# debug and release flags in the compiler and linker  
apply_framework_compile_options_to_target(PVRShell)