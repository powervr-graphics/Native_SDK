# This file provides various functions used by the PowerVR SDK build files
set(FUNCTIONS_INTERNAL_DIR ${CMAKE_CURRENT_LIST_DIR} CACHE INTERNAL "")

include(${CMAKE_CURRENT_LIST_DIR}/CompilerFunctions.cmake)

# Checks to see if the subdirectory exists, and has not already been included
# Parameters: Target, folder to add, binary folder (optional)
function(add_subdirectory_if_exists TARGET SUBDIR_FOLDER)
	get_filename_component(SUBDIR_ABS_PATH ${SUBDIR_FOLDER} ABSOLUTE)
	if((NOT TARGET ${TARGET}) AND EXISTS ${SUBDIR_ABS_PATH}/CMakeLists.txt)
		add_subdirectory(${SUBDIR_FOLDER} EXCLUDE_FROM_ALL)
	endif()
endfunction(add_subdirectory_if_exists)

# As described. Parameters: Target, folder to add, binary folder (optional)
function(add_subdirectory_if_not_already_included TARGET SUBDIR_FOLDER)
	if(NOT TARGET ${TARGET})
		add_subdirectory(${SUBDIR_FOLDER} EXCLUDE_FROM_ALL)
	endif()
endfunction(add_subdirectory_if_not_already_included)

function(download_external_project external_project_name external_project_cmake_files_dir external_project_src_dir external_project_download_dir external_project_url external_project_byproducts)
	# See here for details: https://crascit.com/2015/07/25/cmake-gtest/
	file(COPY ${FUNCTIONS_INTERNAL_DIR}/external_project_download.cmake.in DESTINATION ${external_project_cmake_files_dir} NO_SOURCE_PERMISSIONS)
	configure_file(${external_project_cmake_files_dir}/external_project_download.cmake.in ${external_project_cmake_files_dir}/CMakeLists.txt)

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