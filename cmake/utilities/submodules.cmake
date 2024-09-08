cmake_minimum_required(VERSION 3.10)

# Searches the SUBMODULE_PATH paramater to make sure the directory exists first. 
# External dependencies are managed by a SUBMODULE_PATH/CMakeLists.txt with the 
# submodule itself placed in SUBMODULE_PATH/source/
# This is so we can have control over the submodules cmake system 
function(add_submodule_to_sdk SUBMODULE_PATH)
	
	# First check that the submodule follows the sdk's expected layout
	if(NOT EXISTS ${SUBMODULE_PATH}/source)
		message(FATAL_ERROR "${SUBMODULE_PATH}/source not found! Submodules must be placed in a /source folder")
	endif()

	# Get the name of the submodule by getting the last directory name
	get_filename_component(SUBMODULE_NAME "${SUBMODULE_PATH}" NAME)

	# Now check if there is a .git file, this confirms that the submodule has been 
	# pulled from github
	if(NOT EXISTS ${SUBMODULE_PATH}/source/.git)
		message("Submodule ${SUBMODULE_NAME} not initalised, Attempting to fetch now")

		# Find the git command once and only once
		if(NOT GIT_FOUND)
			find_package(Git QUIET)
			if(NOT GIT_FOUND)
				# Not found git after searching
				message(FATAL_ERROR "Git not found! run : \"git submodule update --init --recursive\"")
			else()
				# Found git, now cache the git variables
				set(GIT_EXECUTABLE "${GIT_EXECUTABLE}" CACHE INTERNAL "Path to the git executable for submodule downloading")
				set(GIT_FOUND "${GIT_FOUND}" CACHE INTERNAL "Boolean telling CMake that git executable was found")
			endif()
		endif()

		# At this point we know that git executable has been found
		execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive source
			WORKING_DIRECTORY ${SUBMODULE_PATH}
			RESULT_VARIABLE GIT_SUBMODULE_CMD_RESULT)

		# Check the command was successful 
		if(NOT GIT_SUBMODULE_CMD_RESULT EQUAL "0")
			message(FATAL_ERROR "Failed updating git submodule ${SUBMODULE_NAME} with error ${GIT_SUBMODULE_CMD_RESULT}")
		endif()

		# One last check that the folder was updated
		if(NOT EXISTS ${SUBMODULE_PATH}/source/.git)
			message(FATAL_ERROR "Submodule ${SUBMODULE_NAME} failed to update, error code was quiet")
		endif()
	endif()

	# We only want a submodule to be added once per build. So if the target already exists then return 
	if(TARGET ${SUBMODULE_NAME})
		return()
	endif()

	# We now know that the submodule has been pulled
	# So run the CMakeLists.txt in the submodules directory
	message(STATUS "Adding Submodule ${SUBMODULE_NAME}")
	add_subdirectory(${SUBMODULE_PATH} ${CMAKE_BINARY_DIR}/${SUBMODULE_NAME})
endfunction()