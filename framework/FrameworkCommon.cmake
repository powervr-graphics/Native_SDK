# This file provides all of the information that is required for independent parts of the framework to be able to build 
# through cmake, this primarily focuses on ensuring that all modules know where the root of the SDK is and also 
# how to find different submodules

# Add a cmake function which streamlines adding submodues to the SDK
include(${CMAKE_CURRENT_LIST_DIR}/../cmake/utilities/submodules.cmake)

# Add a cmake function which adds the default framework compile definitions
include(${CMAKE_CURRENT_LIST_DIR}/../cmake/utilities/compile_options.cmake)

if (NOT ("${CMAKE_CURRENT_LIST_DIR}/cmake/modules" IN_LIST CMAKE_MODULE_PATH))
	# CMAKE_MODULE_PATH is used by find_package
	set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../cmake/modules" CACHE STRING "" FORCE)
endif()

# Define Root paths for consistency across modules
set(PVR_FRAMEWORK_ROOT "${CMAKE_CURRENT_LIST_DIR}")
set(PVR_SDK_ROOT "${CMAKE_CURRENT_LIST_DIR}/..")
set(PVR_EXTERNAL_ROOT "${PVR_SDK_ROOT}/external")

# Common installation function to reduce boilerplate
function(pvr_install_target)
	set(options)
	set(oneValueArgs TARGET)
	set(multiValueArgs HEADERS)
	cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	if(NOT ARG_TARGET)
		message(FATAL_ERROR "pvr_install_target requires TARGET argument")
	endif()

    # Add Namespace Alias (Modern CMake practice)
    if(NOT TARGET PVR::${ARG_TARGET})
        add_library(PVR::${ARG_TARGET} ALIAS ${ARG_TARGET})
    endif()

	if(PVR_ENABLE_INSTALL)
		install(TARGETS ${ARG_TARGET} EXPORT ${ARG_TARGET}Targets
			ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
			LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
			RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
			INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
			OPTIONAL)

		if(ARG_HEADERS)
			foreach(file ${ARG_HEADERS})
				get_filename_component(dir ${file} DIRECTORY)
				install(FILES ${file} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${ARG_TARGET}/${dir})
			endforeach()
		endif()

		install(EXPORT ${ARG_TARGET}Targets DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${ARG_TARGET} OPTIONAL)

        # Handle Config file if it exists
        set(CONFIG_FILE_IN "cmake/${ARG_TARGET}Config.cmake.in")
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${CONFIG_FILE_IN}")
            # Build tree export
            export(EXPORT ${ARG_TARGET}Targets FILE "${CMAKE_CURRENT_BINARY_DIR}/${ARG_TARGET}/${ARG_TARGET}Targets.cmake")
            
            # Configure package config file
            configure_file("${CONFIG_FILE_IN}" "${CMAKE_CURRENT_BINARY_DIR}/${ARG_TARGET}/${ARG_TARGET}Config.cmake" COPYONLY)
            
            # Install config and targets
            install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${ARG_TARGET}/${ARG_TARGET}Config.cmake" 
                    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${ARG_TARGET} OPTIONAL)
            install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${ARG_TARGET}/${ARG_TARGET}Targets.cmake" 
                    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${ARG_TARGET} OPTIONAL)
        endif()
	endif()
endfunction()
