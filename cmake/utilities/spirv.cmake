cmake_minimum_required(VERSION 3.10)

# Adds a shader that should be compiled to spirv.
# Will create a rule that, at build time, will compile the provided shaders from at BASE_PATH/RELATIVE_PATH into spirv at ASSET_FOLDER/RELATIVE_PATH.
# source group for the generated spirv will be SOURCE_GROUP_generated
# Will also add the spirv generated to the resource list of TARGET_NAME (windows)
#  Usage: add_spirv_shaders_to_target(<TARGET_NAME> <SOURCE_GROUP> <ASSET_FOLDER> <BASE_PATH> <RELATIVE_PATH> <VULKAN_TARGET_ENV> [GLSLANG_ARGUMENTS <glslang_arguments>])
#  If the optional GLSLANG_ARGUMENTS argument is provided, it will be passed as the argument to glslangValidator when compiling spir-v
# After adding all assets, remember to also call add_assets_resource_file(TARGET_NAME)
function(add_spirv_shaders_to_target TARGET_NAME)
	cmake_parse_arguments(ARGUMENTS "" "GLSLANG_ARGUMENTS;SOURCE_GROUP;SPIRV_SOURCE_GROUP;BASE_PATH;ASSET_FOLDER;SPIRV_OUTPUT_FOLDER;VULKAN_TARGET_ENV" "FILE_LIST" ${ARGN})
	if(NOT ARGUMENTS_SPIRV_SOURCE_GROUP)
		set(ARGUMENTS_SPIRV_SOURCE_GROUP shaders_generated)
	endif()
	if(NOT ARGUMENTS_SOURCE_GROUP)
		set(ARGUMENTS_SOURCE_GROUP shaders_source)
	endif()
	if(NOT ARGUMENTS_GLSLANG_ARGUMENTS)
		set(ARGUMENTS_GLSLANG_ARGUMENTS -V)
	endif()
	if(NOT ARGUMENTS_SPIRV_OUTPUT_FOLDER)
		if(ANDROID)
			set(ARGUMENTS_SPIRV_OUTPUT_FOLDER ${ARGUMENTS_BASE_PATH})
		else()
			set(ARGUMENTS_SPIRV_OUTPUT_FOLDER ${CMAKE_CURRENT_BINARY_DIR})
		endif()
	endif()
	if(NOT ARGUMENTS_VULKAN_TARGET_ENV)
		set(ARGUMENTS_VULKAN_TARGET_ENV vulkan1.0)
	endif()

	mandatory_args(add_spirv_shaders_to_target BASE_PATH ASSET_FOLDER FILE_LIST)
	unknown_args(add_spirv_shaders_to_target "${ARGUMENTS_UNPARSED_ARGUMENTS}")

	#add the source shader files to source groups. Don't package with the app
	add_assets_to_target(${TARGET_NAME}
		SOURCE_GROUP ${ARGUMENTS_SOURCE_GROUP}
		ASSET_FOLDER ${ARGUMENTS_ASSET_FOLDER}
		BASE_PATH ${ARGUMENTS_BASE_PATH}
		FILE_LIST ${ARGUMENTS_FILE_LIST}
		NO_PACKAGE)

	add_trailing_slash(ARGUMENTS_ASSET_FOLDER)
	add_trailing_slash(ARGUMENTS_BASE_PATH)
	add_trailing_slash(ARGUMENTS_SPIRV_OUTPUT_FOLDER)

	unset(TMP_SHADER_LIST)

	foreach(RELATIVE_PATH ${ARGUMENTS_FILE_LIST})
		LIST(APPEND TMP_SHADER_LIST "${RELATIVE_PATH}.spv")
		get_filename_component(SHADER_NAME ${RELATIVE_PATH} NAME)
		get_glslang_validator_type(SHADER_TYPE SHADER_NAME)
		set (GLSLANG_VALIDATOR_COMPILE_CURRENT_COMMAND $<TARGET_FILE:glslangValidator> ${ARGUMENTS_GLSLANG_ARGUMENTS} --target-env ${ARGUMENTS_VULKAN_TARGET_ENV} ${ARGUMENTS_BASE_PATH}${RELATIVE_PATH} -o ${ARGUMENTS_SPIRV_OUTPUT_FOLDER}${RELATIVE_PATH}.spv -S ${SHADER_TYPE})
		add_custom_command(
			DEPENDS glslangValidator
			OUTPUT ${ARGUMENTS_SPIRV_OUTPUT_FOLDER}/${RELATIVE_PATH}.spv 
			PRE_BUILD 
			MAIN_DEPENDENCY ${ARGUMENTS_BASE_PATH}/${RELATIVE_PATH}
			COMMAND echo ${GLSLANG_VALIDATOR_COMPILE_CURRENT_COMMAND}
			COMMAND ${GLSLANG_VALIDATOR_COMPILE_CURRENT_COMMAND}
			COMMENT "${PROJECT_NAME}: Compiling ${RELATIVE_PATH} to ${RELATIVE_PATH}.spv"
		)
	endforeach()
	
	#add the final spirv-list of files as resources and package them with the app
	add_assets_to_target(${TARGET_NAME}
		SOURCE_GROUP ${ARGUMENTS_SPIRV_SOURCE_GROUP}
		ASSET_FOLDER ${ARGUMENTS_ASSET_FOLDER}
		BASE_PATH ${ARGUMENTS_SPIRV_OUTPUT_FOLDER}
		FILE_LIST ${TMP_SHADER_LIST}
		)
endfunction()

# Helper which takes the file extension of the shader and associates it with 
# a shader type
function(get_glslang_validator_type out_glslang_validator_type INPUT_SHADER_NAME)
	if(${INPUT_SHADER_NAME} MATCHES ".fsh$")
		set(${out_glslang_validator_type} frag PARENT_SCOPE)
	elseif(${INPUT_SHADER_NAME} MATCHES ".vsh$")
		set(${out_glslang_validator_type} vert PARENT_SCOPE)
	elseif(${INPUT_SHADER_NAME} MATCHES ".csh$")
		set(${out_glslang_validator_type} comp PARENT_SCOPE)
	elseif(${INPUT_SHADER_NAME} MATCHES ".gsh$")
		set(${out_glslang_validator_type} geom PARENT_SCOPE)
	elseif(${INPUT_SHADER_NAME} MATCHES ".frag$")
		set(${out_glslang_validator_type} frag PARENT_SCOPE)
	elseif(${INPUT_SHADER_NAME} MATCHES ".vert$")
		set(${out_glslang_validator_type} vert PARENT_SCOPE)
	elseif(${INPUT_SHADER_NAME} MATCHES ".comp$")
		set(${out_glslang_validator_type} comp PARENT_SCOPE)
	elseif(${INPUT_SHADER_NAME} MATCHES ".tcsh$")
		set(${out_glslang_validator_type} tesc PARENT_SCOPE)
	elseif(${INPUT_SHADER_NAME} MATCHES ".tesh$")
		set(${out_glslang_validator_type} tese PARENT_SCOPE)
	elseif(${INPUT_SHADER_NAME} MATCHES ".geom$")
		set(${out_glslang_validator_type} geom PARENT_SCOPE)
	elseif(${INPUT_SHADER_NAME} MATCHES ".mesh$")
		set(${out_glslang_validator_type} mesh PARENT_SCOPE)
	elseif(${INPUT_SHADER_NAME} MATCHES ".rgen$")
		set(${out_glslang_validator_type} rgen PARENT_SCOPE)
	elseif(${INPUT_SHADER_NAME} MATCHES ".rint$")
		set(${out_glslang_validator_type} rint PARENT_SCOPE)
	elseif(${INPUT_SHADER_NAME} MATCHES ".rahit$")
		set(${out_glslang_validator_type} rahit PARENT_SCOPE)
	elseif(${INPUT_SHADER_NAME} MATCHES ".rchit$")
		set(${out_glslang_validator_type} rchit PARENT_SCOPE)
	elseif(${INPUT_SHADER_NAME} MATCHES ".rmiss$")
		set(${out_glslang_validator_type} rmiss PARENT_SCOPE)
	elseif(${INPUT_SHADER_NAME} MATCHES ".rcall$")
		set(${out_glslang_validator_type} rcall PARENT_SCOPE)
	endif()
endfunction(get_glslang_validator_type)