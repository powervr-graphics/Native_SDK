cmake_minimum_required(VERSION 3.10)

set(SDK_ROOT_INTERNAL_DIR ${CMAKE_CURRENT_LIST_DIR}/../.. CACHE INTERNAL "")

option(PVR_ENABLE_EXAMPLE_RECOMMENDED_WARNINGS "If enabled, pass /W4 to the compiler and disable specific unreasonable warnings." ON)
option(PVR_ENABLE_EXAMPLE_FAST_MATH "If enabled, attempt to enable fast-math." ON)

# Ensure the examples can find PVRVFrame OpenGL ES Emulation libraries.
if(WIN32)
	if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
		set(PVR_VFRAME_LIB_FOLDER "${SDK_ROOT_INTERNAL_DIR}/lib/Windows_x86_64" CACHE INTERNAL "")
	else()
		set(PVR_VFRAME_LIB_FOLDER "${SDK_ROOT_INTERNAL_DIR}/lib/Windows_x86_32" CACHE INTERNAL "")
	endif()
elseif(APPLE AND NOT IOS)
	set(PVR_VFRAME_LIB_FOLDER "${SDK_ROOT_INTERNAL_DIR}/lib/macOS_x86" CACHE INTERNAL "")
elseif(UNIX)
	set(PVR_VFRAME_LIB_FOLDER "${SDK_ROOT_INTERNAL_DIR}/lib/${CMAKE_SYSTEM_NAME}_${CMAKE_SYSTEM_PROCESSOR}" CACHE INTERNAL "")
endif()

# Adds an asset to the resource list of TARGET_NAME (windows), and/or add rules to copy it to the asset folder of that target (unix etc.)
# "Assets" in this context are files that we will make available for runtime use to the target. For example, for Windows we can embed them
# into the executable or/end copy them to an asset folder, for UNIX we are copying them to an asset folder, for iOS/MacOS add them to the
# application bundle, for android they are copied into the assets/ folder in the .apk, etc.
# The assets will be copied from BASE_PATH/RELATIVE_PATH, and will be added as resources to the RELATIVE_PATH id for windows, or copied into
# ASSET_FOLDER/RELATIVE_PATH for linux.
#  Usage: add_assets_to_target(<TARGET_NAME> 
#    SOURCE_GROUP <SOURCE_GROUP> 
#    BASE_PATH <BASE_PATH> 
#    ASSET_FOLDER <ASSET_FOLDER> 
#    FILE_LIST <LIST OF RELATIVE_PATH>...)
#After adding all assets, remember to also call add_assets_resource_file(TARGET_NAME)

function(add_assets_to_target TARGET_NAME)
	cmake_parse_arguments(ARGUMENTS "NO_PACKAGE" "SOURCE_GROUP;BASE_PATH;ASSET_FOLDER" "FILE_LIST" ${ARGN})
	mandatory_args(add_assets_to_target BASE_PATH ASSET_FOLDER FILE_LIST)
	unknown_args(add_assets_to_target "${ARGUMENTS_UNPARSED_ARGUMENTS}")

	if(NOT SOURCE_GROUP)
		set(SOURCE_GROUP "assets`")
	endif()
	
	#Add list of all resource files to our target for later use
	add_trailing_slash(ARGUMENTS_ASSET_FOLDER)
	add_trailing_slash(ARGUMENTS_BASE_PATH)
	
	foreach(RELATIVE_PATH ${ARGUMENTS_FILE_LIST})
		set(SOURCE_PATH ${ARGUMENTS_BASE_PATH}${RELATIVE_PATH})
		set(TARGET_PATH ${ARGUMENTS_ASSET_FOLDER}${RELATIVE_PATH})

		source_group(${ARGUMENTS_SOURCE_GROUP} FILES ${SOURCE_PATH})
		target_sources(${TARGET_NAME} PUBLIC ${SOURCE_PATH})

		if (NOT ARGUMENTS_NO_PACKAGE)
			set_property(TARGET ${TARGET_NAME} APPEND PROPERTY SDK_RESOURCE_FILES "${RELATIVE_PATH}|${SOURCE_PATH}")
			if (APPLE)
				source_group(Resources FILES ${SOURCE_PATH})
				set_source_files_properties(${SOURCE_PATH} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
				set_target_properties(${TARGET_NAME} PROPERTIES RESOURCE ${SOURCE_PATH})
			endif()
			get_target_property(IS_COMMAND_LINE ${TARGET_NAME} COMMAND_LINE)
			if((UNIX OR QNX) AND NOT APPLE AND (IS_COMMAND_LINE OR NOT ANDROID)) # Only copy assets for Unix or Android Command line
				#add a rule to copy the asset
				add_custom_command(
					OUTPUT ${TARGET_PATH} 
					PRE_BUILD 
					MAIN_DEPENDENCY ${SOURCE_PATH}
					COMMAND ${CMAKE_COMMAND} -E remove -f ${TARGET_PATH}
					COMMAND ${CMAKE_COMMAND} -E copy ${SOURCE_PATH} ${TARGET_PATH}
					COMMENT "${CMAKE_PROJECT_NAME}: Copying ${SOURCE_PATH} to ${TARGET_PATH}"
				)
			endif()
		endif()
	endforeach()
endfunction()

# Adds a (windows or other resource-based platform) resource file with the resources that were previously added to the target with "add_assets_to_target"
# or "add_spirv_shaders_to_target". It reads a special target property added by those functions: SDK_RESOURCE_FILES, and populates Resources.rc with those files
# This property, if you wish to set it manually, is a list of pipe - separated resource name/resource path pairs, for example: 
# "CarAsset/car.gltf|e:/myfolder/CarAsset/car.gltf;custom_resource_id|e:/anotherfolder/myresource.png..."
function(add_assets_resource_file TARGET_NAME)
	get_property(SDK_RES_LIST TARGET ${TARGET_NAME} PROPERTY SDK_RESOURCE_FILES) #Shorten typing. Global defined/reset by add_platform_specific_executable and populated by add_asset.

	if(WIN32)
		unset(RESOURCE_LIST)
		foreach(TEMP ${SDK_RES_LIST}) #prepare a list in the format we need for Windows resource list
			# Split up the '|' separated name/path pairs, set them in the variable expected by resources.rc.in and populate it
			string(REPLACE "|" ";" PAIR ${TEMP})
			list(GET PAIR 0 RESOURCE_NAME)
			list(GET PAIR 1 RESOURCE_PATH)
			get_filename_component(RESOURCE_PATH ${RESOURCE_PATH} ABSOLUTE)
			file(TO_NATIVE_PATH "${RESOURCE_PATH}" RESOURCE_PATH)
			string(REPLACE "\\" "\\\\" RESOURCE_PATH "${RESOURCE_PATH}")
			set(RESOURCE_LIST ${RESOURCE_LIST} "${RESOURCE_NAME} RCDATA \"${RESOURCE_PATH}\"\n")
		endforeach()

		if (NOT(RESOURCE_LIST STREQUAL ""))
			#flatten the list into a string that will be the whole resource block of the file
			string(REPLACE ";" "" RESOURCE_LIST "${RESOURCE_LIST}")
			configure_file(${SDK_ROOT_INTERNAL_DIR}/res/Windows/Resources.rc.in ${CMAKE_CURRENT_BINARY_DIR}/cmake-resources/Resources.rc)
			#Add the resource files needed for windows (icons, asset files etc).
			target_sources(${TARGET_NAME} PUBLIC ${SOURCE_PATH} "${CMAKE_CURRENT_BINARY_DIR}/cmake-resources/Resources.rc")
		endif()
	endif()
endfunction()


function(add_trailing_slash MYPATH)
	if (NOT ${${MYPATH}} MATCHES "/$")
		set(${MYPATH} "${${MYPATH}}/" PARENT_SCOPE)
	endif()
endfunction()

function(mandatory_args FUNCTION_NAME ARG_NAMES)
	foreach(ARG_NAME ${ARG_NAMES})
		if(NOT ${ARG_NAME} AND NOT ARGUMENTS_${ARG_NAME})
			message(FATAL_ERROR "Function ${FUNCTION_NAME}: Mandatory parameter ${ARG_NAME} not defined")
		endif()
	endforeach()
endfunction()

function(unknown_args FUNCTION_NAME ARG_NAMES)
	if(ARG_NAMES)
		message(FATAL_ERROR "Function ${FUNCTION_NAME}: Unknown parameters passed: ${ARG_NAMES}")
	endif()
endfunction()