
function (copy_file_if_changed input output)
	add_custom_command(OUTPUT ${output} COMMAND ${CMAKE_COMMAND} -E copy ${input} ${output} MAIN_DEPENDENCY ${input})
endfunction(copy_file_if_changed)

function (add_subdirectory_if_not_already_included TARGET SUBDIR_FOLDER SUBDIR_BIN_FOLDER)
	if (NOT TARGET ${TARGET})
		add_subdirectory(${SUBDIR_FOLDER} ${SUBDIR_BIN_FOLDER})
	endif()
endfunction(add_subdirectory_if_not_already_included)

macro (add_platform_specific_resource_files SRC_FILES RESOURCE_FILES)
	if (WIN32)
		list (APPEND SRC_FILES  #Add the resource files needed for windows (icons, asset files etc).
		"${SDK_ROOT}/res/Windows/shared.rc"
		"${SDK_ROOT}/res/Windows/resource.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/cmake-resources/Resources.rc")
	elseif(APPLE)
		if (IOS)
			set(INFO_PLIST_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cmake-resources/iOS_Info.plist")
			file(GLOB ICONS LIST_DIRECTORIES false ${SDK_ROOT}/res/iOS/* ${SDK_ROOT}/res/iOS/OpenGLES/*)
			list(APPEND RESOURCE_FILES ${ICONS})
		else()
			set(INFO_PLIST_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cmake-resources/macOS_Info.plist")
			list(APPEND FRAMEWORK_FILES "${EXTERNAL_LIB_FOLDER}/libEGL.dylib" "${EXTERNAL_LIB_FOLDER}/libGLESv2.dylib")
			set_source_files_properties(${FRAMEWORK_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Frameworks)
			source_group(Frameworks FILES ${FRAMEWORK_FILES})
			list(APPEND RESOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/cmake-resources/MainMenu.xib")
		endif()
		source_group(Resources FILES ${RESOURCE_FILES})
		set_source_files_properties(${RESOURCE_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
	endif()
endmacro()

# CAUTION - For this rule to work, the asset files must actually be added as sources to the executable
macro(add_rule_copy_assets_to_asset_folder RESOURCE_FILES OUTPUT_FOLDER)
	if (UNIX OR QNX)
		#Copy all assets to the Assets folder in order for the executable to be able to locate it
		foreach (ASSET_FILE_PATH ${RESOURCE_FILES})
			get_filename_component(ASSET_FILE_NAME ${ASSET_FILE_PATH} NAME)
			add_custom_command(
				OUTPUT ${OUTPUT_FOLDER}/${ASSET_FILE_NAME} 
				PRE_BUILD 
				MAIN_DEPENDENCY ${ASSET_FILE_PATH} 
				COMMAND ${CMAKE_COMMAND} -E copy ${ASSET_FILE_PATH} ${OUTPUT_FOLDER}/${ASSET_FILE_NAME}
				COMMENT "${CMAKE_PROJECT_NAME}: Copying ${ASSET_FILE_PATH} to ${OUTPUT_FOLDER}/${ASSET_FILE_NAME}"
				)
		endforeach()
	endif() #All other platforms package: Windows resources, MacOS package, Android Assets etc.
endmacro()

function(get_glslang_validator_type out_glslang_validator_type SHADER_NAME)
		if (${SHADER_NAME} MATCHES ".fsh$")
			set(${out_glslang_validator_type} frag PARENT_SCOPE)
		elseif (${SHADER_NAME} MATCHES ".vsh$")
			set(${out_glslang_validator_type} vert PARENT_SCOPE)
		elseif (${SHADER_NAME} MATCHES ".csh$")
			set(${out_glslang_validator_type} comp PARENT_SCOPE)
		elseif (${SHADER_NAME} MATCHES ".gsh$")
			set(${out_glslang_validator_type} geom PARENT_SCOPE)
		elseif (${SHADER_NAME} MATCHES ".tcsh$")
			set(${out_glslang_validator_type} tesc PARENT_SCOPE)
		elseif (${SHADER_NAME} MATCHES ".tesh$")
			set(${out_glslang_validator_type} tese PARENT_SCOPE)
		endif()
endfunction(get_glslang_validator_type)

function(add_rule_generate_spirv_from_shaders SHADER_NAMES)
	#GENERATE A PRE-BUILD STEP FOR COMPILING GLSL TO SPIRV
	foreach (SHADER ${SHADER_NAMES})
		get_filename_component(SHADER_NAME ${SHADER} NAME)
		get_glslang_validator_type(SHADER_TYPE ${SHADER})
		set (SPIRV_COMPILE_CURRENT_COMMAND ${SPIRV_COMPILER} -V ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER_NAME} -o ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER_NAME}.spv -S ${SHADER_TYPE})
		add_custom_command(
			OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER_NAME}.spv 
			PRE_BUILD 
			MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER_NAME}
			COMMAND echo ${SPIRV_COMPILE_CURRENT_COMMAND} && ${SPIRV_COMPILE_CURRENT_COMMAND}
			COMMENT "${CMAKE_PROJECT_NAME}: Compiling ${SHADER_NAME} to ${SHADER_NAME}.spv"
			)
	endforeach()
endfunction(add_rule_generate_spirv_from_shaders)