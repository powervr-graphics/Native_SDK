cmake_minimum_required(VERSION 3.3)
project (PVRPfx)

set(PVRPfx_SRC
	   ${CMAKE_CURRENT_LIST_DIR}/EffectVk.cpp
	   ${CMAKE_CURRENT_LIST_DIR}/EffectVk.h
	   ${CMAKE_CURRENT_LIST_DIR}/RenderManagerVk.cpp
	   ${CMAKE_CURRENT_LIST_DIR}/RenderManagerVk.h)

set(PVRPfx_LINK_LIBS PowerVR_SDK PVRUtilsVk)
	   
get_directory_property(HAS_PARENT PARENT_DIRECTORY)
if(HAS_PARENT AND USE_PREBUILT_DEPENDENCIES)
add_library(PVRPfx STATIC IMPORTED GLOBAL)

# add the subdirectory after the add_library so that the target exists
add_subdirectory(../.. ${CMAKE_CURRENT_BINARY_DIR}/sdk EXCLUDE_FROM_ALL)

set_target_properties(PVRPfx PROPERTIES
	IMPORTED_LOCATION_DEBUG ${FRAMEWORK_LIB_FOLDER}/${CMAKE_STATIC_LIBRARY_PREFIX}PVRPfx${CMAKE_STATIC_LIBRARY_SUFFIX}
	IMPORTED_LOCATION_RELEASE ${FRAMEWORK_LIB_FOLDER}/${CMAKE_STATIC_LIBRARY_PREFIX}PVRPfx${CMAKE_STATIC_LIBRARY_SUFFIX}
	INTERFACE_LINK_LIBRARIES "${PVRPfx_LINK_LIBS}")
else()
add_library(PVRPfx STATIC EXCLUDE_FROM_ALL ${PVRPfx_SRC})

# add the subdirectory after the add_library so that the target exists
add_subdirectory(../.. ${CMAKE_CURRENT_BINARY_DIR}/sdk EXCLUDE_FROM_ALL)

target_link_libraries(PVRPfx ${PVRPfx_LINK_LIBS})
set_target_properties(PVRPfx PROPERTIES 
	ARCHIVE_OUTPUT_DIRECTORY ${FRAMEWORK_LIB_FOLDER}
	LIBRARY_OUTPUT_DIRECTORY ${FRAMEWORK_LIB_FOLDER})
endif()