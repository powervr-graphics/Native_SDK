# This file provides various functions used by the PowerVR SDK build files
set(FUNCTIONS_INTERNAL_DIR ${CMAKE_CURRENT_LIST_DIR} CACHE INTERNAL "")

function(add_subdirectory_if_not_already_included TARGET SUBDIR_FOLDER SUBDIR_BIN_FOLDER)
	if (NOT TARGET ${TARGET})
		add_subdirectory(${SUBDIR_FOLDER} ${SUBDIR_BIN_FOLDER} EXCLUDE_FROM_ALL)
	endif()
endfunction(add_subdirectory_if_not_already_included)

function(add_platform_specific_resource_files INPUT_SRC_FILES INPUT_RESOURCE_FILES)
	if (WIN32)
		set(RESOURCE_LIST "")
		foreach(RESOURCE ${${INPUT_RESOURCE_FILES}})
			get_filename_component(RESOURCE_NAME ${RESOURCE} NAME)
			file(RELATIVE_PATH RESOURCE ${CMAKE_CURRENT_SOURCE_DIR}/cmake-resources ${RESOURCE})
			file(TO_NATIVE_PATH "${RESOURCE}" RESOURCE)
			string(REPLACE "\\" "\\\\" RESOURCE "${RESOURCE}")
			set(RESOURCE_LIST ${RESOURCE_LIST} "${RESOURCE_NAME} RCDATA \"${RESOURCE}\"\n")
		endforeach()
		string(REPLACE ";" "" RESOURCE_LIST "${RESOURCE_LIST}")
		configure_file(${SDK_ROOT}/res/Windows/Resources.rc.in ${CMAKE_CURRENT_SOURCE_DIR}/cmake-resources/Resources.rc)
		#Add the resource files needed for windows (icons, asset files etc).
		list(APPEND ${INPUT_SRC_FILES}
			"${SDK_ROOT}/res/Windows/shared.rc"
			"${SDK_ROOT}/res/Windows/resource.h"
			"${CMAKE_CURRENT_SOURCE_DIR}/cmake-resources/Resources.rc")
	elseif(APPLE)
		if (IOS)
			set(INFO_PLIST_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cmake-resources/iOS_Info.plist" PARENT_SCOPE)
			file(GLOB ICONS LIST_DIRECTORIES false ${SDK_ROOT}/res/iOS/* ${SDK_ROOT}/res/iOS/OpenGLES/*)
			list(APPEND ${INPUT_RESOURCE_FILES} ${ICONS})
		else()
			set(INFO_PLIST_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cmake-resources/macOS_Info.plist" PARENT_SCOPE)
			list(APPEND ${INPUT_RESOURCE_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/cmake-resources/MainMenu.xib")
		endif()
		source_group(Resources FILES ${${INPUT_RESOURCE_FILES}})
		set_source_files_properties(${${INPUT_RESOURCE_FILES}} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
	endif()
	SET(${INPUT_SRC_FILES} ${${INPUT_SRC_FILES}} PARENT_SCOPE)
	SET(${INPUT_RESOURCE_FILES} ${${INPUT_RESOURCE_FILES}} PARENT_SCOPE)
endfunction()

function(add_platform_specific_executable EXECUTABLE_NAME INPUT_SRC_FILES INPUT_RESOURCE_FILES)
	if (WIN32)
		add_executable(${EXECUTABLE_NAME} WIN32 ${INPUT_SRC_FILES})
	elseif (ANDROID)
		add_library(${EXECUTABLE_NAME} SHARED ${INPUT_SRC_FILES})
		# Force export ANativeActivity_onCreate(),
		# Refer to: https://github.com/android-ndk/ndk/issues/381
		set_target_properties(${EXECUTABLE_NAME} PROPERTIES LINK_FLAGS " -u ANativeActivity_onCreate")
	elseif (APPLE)
		if (IOS) 
			add_executable(${EXECUTABLE_NAME} MACOSX_BUNDLE ${INPUT_SRC_FILES} ${INPUT_RESOURCE_FILES})
		else ()
			list(APPEND FRAMEWORK_FILES "${EXTERNAL_LIB_FOLDER}/libEGL.dylib" "${EXTERNAL_LIB_FOLDER}/libGLESv2.dylib")
			set_source_files_properties(${FRAMEWORK_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Frameworks)
			source_group(Frameworks FILES ${FRAMEWORK_FILES})
			add_executable(${EXECUTABLE_NAME} MACOSX_BUNDLE ${INPUT_SRC_FILES} ${INPUT_RESOURCE_FILES} ${FRAMEWORK_FILES})
		endif ()
		set_target_properties(${EXECUTABLE_NAME} PROPERTIES RESOURCE "${INPUT_RESOURCE_FILES}")
		set_target_properties(${EXECUTABLE_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${INFO_PLIST_FILE}")
	elseif (UNIX OR QNX)
		add_executable(${EXECUTABLE_NAME} ${INPUT_SRC_FILES})
	endif()
endfunction()

# CAUTION - For this rule to work, the asset files must actually be added as sources to the executable
function(add_rule_copy_assets_to_asset_folder INPUT_RESOURCE_FILES INPUT_OUTPUT_FOLDER)
	if (UNIX OR QNX)
		#Copy all assets to the Assets folder in order for the executable to be able to locate it
		foreach(ASSET_FILE_PATH ${INPUT_RESOURCE_FILES})
			get_filename_component(ASSET_FILE_NAME ${ASSET_FILE_PATH} NAME)
			add_custom_command(
				OUTPUT ${INPUT_OUTPUT_FOLDER}/${ASSET_FILE_NAME} 
				PRE_BUILD 
				MAIN_DEPENDENCY ${ASSET_FILE_PATH} 
				COMMAND ${CMAKE_COMMAND} -E copy ${ASSET_FILE_PATH} ${INPUT_OUTPUT_FOLDER}/${ASSET_FILE_NAME}
				COMMENT "${CMAKE_PROJECT_NAME}: Copying ${ASSET_FILE_PATH} to ${INPUT_OUTPUT_FOLDER}/${ASSET_FILE_NAME}"
				)
		endforeach()
	endif() #All other platforms package: Windows resources, MacOS package, Android Assets etc.
endfunction()

function(get_glslang_validator_type out_glslang_validator_type INPUT_SHADER_NAME)
	if(${INPUT_SHADER_NAME} MATCHES ".fsh$")
		set(${out_glslang_validator_type} frag PARENT_SCOPE)
	elseif(${INPUT_SHADER_NAME} MATCHES ".vsh$")
		set(${out_glslang_validator_type} vert PARENT_SCOPE)
	elseif(${INPUT_SHADER_NAME} MATCHES ".csh$")
		set(${out_glslang_validator_type} comp PARENT_SCOPE)
	elseif(${INPUT_SHADER_NAME} MATCHES ".gsh$")
		set(${out_glslang_validator_type} geom PARENT_SCOPE)
	elseif(${INPUT_SHADER_NAME} MATCHES ".tcsh$")
		set(${out_glslang_validator_type} tesc PARENT_SCOPE)
	elseif(${INPUT_SHADER_NAME} MATCHES ".tesh$")
		set(${out_glslang_validator_type} tese PARENT_SCOPE)
	endif()
endfunction(get_glslang_validator_type)

function(add_rule_generate_spirv_from_shaders INPUT_SHADER_NAMES)
	#GENERATE A PRE-BUILD STEP FOR COMPILING GLSL TO SPIRV
	foreach(SHADER ${INPUT_SHADER_NAMES})
		get_filename_component(SHADER_NAME ${SHADER} NAME)
		get_glslang_validator_type(SHADER_TYPE SHADER_NAME)
		set (GLSLANG_VALIDATOR_COMPILE_CURRENT_COMMAND glslangValidator -V ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER_NAME} -o ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER_NAME}.spv -S ${SHADER_TYPE})
		add_custom_command(
			DEPENDS glslangValidator
			OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER_NAME}.spv 
			PRE_BUILD 
			MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER_NAME}
			COMMAND echo ${GLSLANG_VALIDATOR_COMPILE_CURRENT_COMMAND}
			COMMAND ${GLSLANG_VALIDATOR_COMPILE_CURRENT_COMMAND}
			COMMENT "${PROJECT_NAME}: Compiling ${SHADER_NAME} to ${SHADER_NAME}.spv"
		)
	endforeach()
endfunction(add_rule_generate_spirv_from_shaders)

function(download_external_project external_project_name external_project_cmake_files_dir external_project_src_dir external_project_download_dir external_project_url external_project_byproducts)
	# See here for details: https://crascit.com/2015/07/25/cmake-gtest/
	configure_file(${FUNCTIONS_INTERNAL_DIR}/external_project_download.cmake ${external_project_cmake_files_dir}/CMakeLists.txt)

	execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" -D "CMAKE_MAKE_PROGRAM:FILE=${CMAKE_MAKE_PROGRAM}" .
					WORKING_DIRECTORY "${external_project_cmake_files_dir}" 
					RESULT_VARIABLE download_configure_result 
					OUTPUT_VARIABLE download_configure_output
					ERROR_VARIABLE download_configure_output)
	if(download_configure_result)
		message(FATAL_ERROR "${external_project_name} download configure step failed (${download_configure_result}): ${download_configure_output}")
	endif()

	execute_process(COMMAND ${CMAKE_COMMAND} --build .
					WORKING_DIRECTORY "${external_project_cmake_files_dir}"
					RESULT_VARIABLE download_build_result 
					OUTPUT_VARIABLE download_build_output
					ERROR_VARIABLE download_build_output)
	if(download_build_result)
		message(FATAL_ERROR "${external_project_name} download build step failed (${download_build_result}): ${download_build_output}")
	endif()
endfunction(download_external_project)