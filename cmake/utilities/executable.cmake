cmake_minimum_required(VERSION 3.10)

# Adds the correct executable file (Windows MSVS/MinGW, Linux, MacOS, iOS or libary(Android)
# Optional parameters:
# COMMAND_LINE: Create a command line application instead of a windowed application. Will
#    not function for iOS. Android command line applications are binaries that should be run
#    through a shell (i.e. are not usable through an APK)
# SKIP_ICON_RESOURCES: Do not add the resource files for icons of the app (on non Windows, no effect)
function(add_platform_specific_executable EXECUTABLE_NAME)
	cmake_parse_arguments("ARGUMENTS" "COMMAND_LINE;SKIP_ICON_RESOURCES" "" "" ${ARGN})
	if(WIN32)
		if(ARGUMENTS_COMMAND_LINE)
			add_executable(${EXECUTABLE_NAME} ${ARGUMENTS_UNPARSED_ARGUMENTS})
		else()
			add_executable(${EXECUTABLE_NAME} WIN32 ${ARGUMENTS_UNPARSED_ARGUMENTS})
		endif()
		if(MINGW)
			# The following fixes "crt0_c.c:(.text.startup+0x2e): undefined reference to `WinMain'" seen when linking with mingw.
			# Without the following mingw fails to find WinMain as it is defined in PVRShell but the symbol has not been marked as an unresolved symbol. The fix is to mark WinMain as unresolved which ensures that WinMain is not stripped. 
			set_target_properties(${EXECUTABLE_NAME} PROPERTIES LINK_FLAGS " -u WinMain")
		endif()
	elseif(ANDROID)
		if(NOT ARGUMENTS_COMMAND_LINE)
			add_library(${EXECUTABLE_NAME} SHARED ${ARGUMENTS_UNPARSED_ARGUMENTS})
			# Force export ANativeActivity_onCreate(),
			# Refer to: https://github.com/android-ndk/ndk/issues/381
			set_target_properties(${EXECUTABLE_NAME} PROPERTIES LINK_FLAGS " -u ANativeActivity_onCreate")
		else()
			#Remember to also ensure Gradle is configured and called in a way suitable for creating a command line example
			add_executable(${EXECUTABLE_NAME} ${ARGUMENTS_UNPARSED_ARGUMENTS})
		endif()
	elseif(APPLE)
		if(IOS) 
			if (ARGUMENTS_COMMAND_LINE)
				message(SEND_ERROR "${EXECUTABLE_NAME}: Command line executables not supported on iOS platforms")
			else()
				configure_file(${SDK_ROOT_INTERNAL_DIR}/res/iOS/Entitlements.plist ${CMAKE_CURRENT_BINARY_DIR}/cmake-resources/Entitlements.plist COPYONLY)
				set(INFO_PLIST_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cmake-resources/iOS_Info.plist")
				file(GLOB EXTRA_RESOURCES LIST_DIRECTORIES false ${SDK_ROOT_INTERNAL_DIR}/res/iOS/* ${SDK_ROOT_INTERNAL_DIR}/res/iOS/OpenGLES/*)
			
				add_executable(${EXECUTABLE_NAME} MACOSX_BUNDLE ${EXTRA_RESOURCES} ${ARGUMENTS_UNPARSED_ARGUMENTS})
				if(CODE_SIGN_IDENTITY)
					set_target_properties (${EXECUTABLE_NAME} PROPERTIES XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "${CODE_SIGN_IDENTITY}")
				endif()
				if(DEVELOPMENT_TEAM_ID)
					set_target_properties (${EXECUTABLE_NAME} PROPERTIES XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "${DEVELOPMENT_TEAM_ID}")
				endif()
			endif()
		else()
			configure_file(${SDK_ROOT_INTERNAL_DIR}/res/macOS/MainMenu.xib.in ${CMAKE_CURRENT_BINARY_DIR}/cmake-resources/MainMenu.xib)
			configure_file(${SDK_ROOT_INTERNAL_DIR}/res/macOS/macOS_Info.plist ${CMAKE_CURRENT_BINARY_DIR}/cmake-resources/macOS_Info.plist COPYONLY)
			set(INFO_PLIST_FILE "${CMAKE_CURRENT_BINARY_DIR}/cmake-resources/macOS_Info.plist")
			list(APPEND EXTRA_RESOURCES "${CMAKE_CURRENT_BINARY_DIR}/cmake-resources/MainMenu.xib")
			
			list(APPEND FRAMEWORK_FILES "${PVR_VFRAME_LIB_FOLDER}/libEGL.dylib" "${PVR_VFRAME_LIB_FOLDER}/libGLESv2.dylib")
			find_package(MoltenVK)
			if(MOLTENVK_FOUND) 
				list(APPEND FRAMEWORK_FILES "${MVK_LIBRARIES}") 
			endif()
			
			set_source_files_properties(${FRAMEWORK_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Frameworks)
			source_group(Frameworks FILES ${FRAMEWORK_FILES})
			add_executable(${EXECUTABLE_NAME} MACOSX_BUNDLE ${EXTRA_RESOURCES} ${ARGUMENTS_UNPARSED_ARGUMENTS} ${FRAMEWORK_FILES})
		endif()
		set_target_properties(${EXECUTABLE_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${INFO_PLIST_FILE}")
		source_group(Resources FILES ${EXTRA_RESOURCES})
		set_source_files_properties(${EXTRA_RESOURCES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

	elseif(UNIX OR QNX)
		add_executable(${EXECUTABLE_NAME} ${ARGUMENTS_UNPARSED_ARGUMENTS})
	endif()
	if(WIN32 AND NOT ARGUMENTS_SKIP_ICON_RESOURCES)
		target_sources(${EXECUTABLE_NAME} PRIVATE
			"${SDK_ROOT_INTERNAL_DIR}/res/Windows/shared.rc"
			"${SDK_ROOT_INTERNAL_DIR}/res/Windows/resource.h")
	endif()
	if (ARGUMENTS_COMMAND_LINE) #Just to use in other places
		set_target_properties(${EXECUTABLE_NAME} PROPERTIES COMMAND_LINE 1)
	endif()
	
	#Add to the global list of targets
	set_property(GLOBAL APPEND PROPERTY PVR_EXAMPLE_TARGETS ${EXECUTABLE_NAME})
endfunction()