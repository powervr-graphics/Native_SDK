cmake_minimum_required(VERSION 3.3)
project (PVRCamera)

set(PVRCamera_SRC
	PVRCamera.h
	CameraInterface.h)

if(ANDROID)
	list(APPEND PVRCamera_SRC CameraInterface_Android.cpp)
elseif (IOS)
		list(APPEND PVRCamera_SRC CameraInterface_iOS.mm)
else()
		list(APPEND PVRCamera_SRC CameraInterface_Dummy.cpp)
endif()

set(PVRCamera_LINK_LIBS PowerVR_SDK PVRCore OpenGLESPlatformInterface)

get_directory_property(HAS_PARENT PARENT_DIRECTORY)
if(HAS_PARENT AND USE_PREBUILT_DEPENDENCIES)
add_library(PVRCamera STATIC IMPORTED GLOBAL)

# add the subdirectory after the add_library so that the target exists
add_subdirectory(../.. ${CMAKE_CURRENT_BINARY_DIR}/sdk EXCLUDE_FROM_ALL)

set_target_properties(PVRCamera PROPERTIES
	IMPORTED_LOCATION_DEBUG ${FRAMEWORK_LIB_FOLDER}/${CMAKE_STATIC_LIBRARY_PREFIX}PVRCamera${CMAKE_STATIC_LIBRARY_SUFFIX}
	IMPORTED_LOCATION_RELEASE ${FRAMEWORK_LIB_FOLDER}/${CMAKE_STATIC_LIBRARY_PREFIX}PVRCamera${CMAKE_STATIC_LIBRARY_SUFFIX}
	INTERFACE_LINK_LIBRARIES "${PVRCamera_LINK_LIBS}")
else()
add_library(PVRCamera STATIC EXCLUDE_FROM_ALL ${PVRCamera_SRC})

# add the subdirectory after the add_library so that the target exists
add_subdirectory(../.. ${CMAKE_CURRENT_BINARY_DIR}/sdk EXCLUDE_FROM_ALL)

target_link_libraries(PVRCamera ${PVRCamera_LINK_LIBS})
set_target_properties(PVRCamera PROPERTIES 
	ARCHIVE_OUTPUT_DIRECTORY ${FRAMEWORK_LIB_FOLDER}
	LIBRARY_OUTPUT_DIRECTORY ${FRAMEWORK_LIB_FOLDER})
endif()