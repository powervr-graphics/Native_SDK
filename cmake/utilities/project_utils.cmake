# Copyright (c) Imagination Technologies Limited.

# Checks to see if the subdirectory exists, and has not already been included
# Sets a convenience variable for it
# Parameters: Target, folder to add
function(add_subdirectory_if_exists TARGET SUBDIR_FOLDER)
	get_filename_component(SUBDIR_ABS_PATH ${SUBDIR_FOLDER} ABSOLUTE)
	if(EXISTS ${SUBDIR_ABS_PATH}/CMakeLists.txt)
		if (NOT TARGET ${TARGET})
			add_subdirectory(${SUBDIR_FOLDER} EXCLUDE_FROM_ALL)
			set("${TARGET}_EXISTS" 1 CACHE INTERNAL "")
		endif()
	else()
		set("${TARGET}_EXISTS" 0 CACHE INTERNAL "")
	endif()
endfunction(add_subdirectory_if_exists)
