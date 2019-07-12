cmake_minimum_required(VERSION 3.3)
project (PVRShell)

list(APPEND PVRShell_INCLUDE_DIRECTORIES "")

set(PVRShell_SRC
	${CMAKE_CURRENT_LIST_DIR}/PVRShell.h
	${CMAKE_CURRENT_LIST_DIR}/Shell.cpp
	${CMAKE_CURRENT_LIST_DIR}/Shell.h
	${CMAKE_CURRENT_LIST_DIR}/ShellData.h
	${CMAKE_CURRENT_LIST_DIR}/StateMachine.cpp
	${CMAKE_CURRENT_LIST_DIR}/StateMachine.h
	${CMAKE_CURRENT_LIST_DIR}/Time.cpp
	${CMAKE_CURRENT_LIST_DIR}/Time_.h
	${CMAKE_CURRENT_LIST_DIR}/OS/ShellOS.h)

#Each platform has its own implementation of PVRShell
if(WIN32)			   
	set(PVRShellOS_SRC
		${CMAKE_CURRENT_LIST_DIR}/OS/Windows/ShellOS.cpp
		${CMAKE_CURRENT_LIST_DIR}/OS/Windows/WindowsOSData.h
		${CMAKE_CURRENT_LIST_DIR}/EntryPoint/WinMain/main.cpp)
elseif (APPLE)
	if (IOS)
		set(PVRShellOS_SRC
			${CMAKE_CURRENT_LIST_DIR}/EntryPoint/UIApplicationMain/AppController.h
			${CMAKE_CURRENT_LIST_DIR}/EntryPoint/UIApplicationMain/AppController.mm
			${CMAKE_CURRENT_LIST_DIR}/EntryPoint/UIApplicationMain/main.m
			${CMAKE_CURRENT_LIST_DIR}/OS/UIKit/ShellOS.mm)
	else()
		set(PVRShellOS_SRC
			${CMAKE_CURRENT_LIST_DIR}/EntryPoint/NSApplicationMain/AppController.h
			${CMAKE_CURRENT_LIST_DIR}/EntryPoint/NSApplicationMain/AppController.mm
			${CMAKE_CURRENT_LIST_DIR}/EntryPoint/NSApplicationMain/main.m
			${CMAKE_CURRENT_LIST_DIR}/OS/AppKit/ShellOS.mm
			${CMAKE_CURRENT_LIST_DIR}/OS/AppKit/ViewMVK.h
			${CMAKE_CURRENT_LIST_DIR}/OS/AppKit/ViewMVK.mm)
	endif()
elseif(ANDROID)
	set(PVRShellOS_SRC
	${CMAKE_CURRENT_LIST_DIR}/EntryPoint/android_main/main.cpp
	${CMAKE_CURRENT_LIST_DIR}/OS/Android/ShellOS.cpp
	${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)
	
	list(APPEND PVRShell_INCLUDE_DIRECTORIES ${ANDROID_NDK}/sources/android/native_app_glue)
elseif(UNIX)
	#Each windowing system has its own implementation of PVRShell
	if ("${WS}" STREQUAL "")
		message("PVRShell:[WS] variable not set: [${WS}].")
		set (WS "NullWS")
		message("PVRShell:[WS] variable now set to: [${WS}]. Otherwise, pass -DWS=[NullWS,X11,Wayland,Screen] as valid values for Unix")
	endif()

	if ("${WS}" STREQUAL "NullWS")
		message("CMake: Generating PVRShell for NullWS")
		if(CMAKE_SYSTEM_NAME MATCHES "QNX")
			set(PVRShellOS_SRC
				${CMAKE_CURRENT_LIST_DIR}/OS/QNXNullWS/ShellOS.cpp
				${CMAKE_CURRENT_LIST_DIR}/EntryPoint/main/main.cpp)
		else()
			set(PVRShellOS_SRC
				${CMAKE_CURRENT_LIST_DIR}/OS/NullWS/ShellOS.cpp
				${CMAKE_CURRENT_LIST_DIR}/EntryPoint/main/main.cpp)
		endif()
	elseif("${WS}" STREQUAL "X11")
		message("CMake: Generating PVRShell for X11")
		set(PVRShellOS_SRC
			${CMAKE_CURRENT_LIST_DIR}/OS/X11/ShellOS.cpp
			${CMAKE_CURRENT_LIST_DIR}/EntryPoint/main/main.cpp)
	elseif("${WS}" STREQUAL "Wayland")
		message("CMake: Generating PVRShell for Wayland")
		set(PVRShellOS_SRC
			${CMAKE_CURRENT_LIST_DIR}/OS/Wayland/ShellOS.cpp
			${CMAKE_CURRENT_LIST_DIR}/EntryPoint/main/main.cpp)
	elseif("${WS}" STREQUAL "Screen")
		if(CMAKE_SYSTEM_NAME MATCHES "QNX")
			message("CMake: Generating PVRShell for Screen")
			set(PVRShellOS_SRC
				${CMAKE_CURRENT_LIST_DIR}/OS/QNXScreen/ShellOS.cpp
				${CMAKE_CURRENT_LIST_DIR}/EntryPoint/main/main.cpp)
		else()
			message(FATAL_ERROR "[WS] variable set to [${WS}] which is only supported on QNX platforms.")
		endif()
	else()
		message(FATAL_ERROR "[WS] variable set to invalid value [${WS}]. Please set WS to a value among: [NullWS, X11, Wayland]")
	endif()
endif()

set(PVRShell_ALL_SRCS ${PVRShell_SRC} ${PVRShellOS_SRC})
set(PVRShell_LINK_LIBS PowerVR_SDK PVRCore WindowSystemInterface)

get_directory_property(HAS_PARENT PARENT_DIRECTORY)
if(HAS_PARENT AND USE_PREBUILT_DEPENDENCIES)

add_library(PVRShell STATIC IMPORTED GLOBAL)

# add the subdirectory after the add_library so that the target exists
add_subdirectory(../.. ${CMAKE_CURRENT_BINARY_DIR}/sdk EXCLUDE_FROM_ALL)

set_target_properties(PVRShell PROPERTIES
	IMPORTED_LOCATION_DEBUG ${FRAMEWORK_LIB_FOLDER}/${CMAKE_STATIC_LIBRARY_PREFIX}PVRShell${CMAKE_STATIC_LIBRARY_SUFFIX}
	IMPORTED_LOCATION_RELEASE ${FRAMEWORK_LIB_FOLDER}/${CMAKE_STATIC_LIBRARY_PREFIX}PVRShell${CMAKE_STATIC_LIBRARY_SUFFIX}
	INTERFACE_LINK_LIBRARIES "${PVRShell_LINK_LIBS}"
	INTERFACE_INCLUDE_DIRECTORIES "${PVRShell_INCLUDE_DIRECTORIES}")
else()
add_library(PVRShell STATIC EXCLUDE_FROM_ALL ${PVRShell_ALL_SRCS})

# add the subdirectory after the add_library so that the target exists
add_subdirectory(../.. ${CMAKE_CURRENT_BINARY_DIR}/sdk EXCLUDE_FROM_ALL)

target_link_libraries(PVRShell ${PVRShell_LINK_LIBS})
target_include_directories(PVRShell PUBLIC ${PVRShell_INCLUDE_DIRECTORIES})
set_target_properties(PVRShell PROPERTIES 
	ARCHIVE_OUTPUT_DIRECTORY ${FRAMEWORK_LIB_FOLDER}
	LIBRARY_OUTPUT_DIRECTORY ${FRAMEWORK_LIB_FOLDER})
endif()