cmake_minimum_required(VERSION 3.3)
project (PVRAssets)

set(PVRAssets_SRC
	${CMAKE_CURRENT_LIST_DIR}/BoundingBox.h
	${CMAKE_CURRENT_LIST_DIR}/fileio/GltfReader.cpp
	${CMAKE_CURRENT_LIST_DIR}/fileio/GltfReader.h
	${CMAKE_CURRENT_LIST_DIR}/fileio/PODDefines.h
	${CMAKE_CURRENT_LIST_DIR}/fileio/PODReader.cpp
	${CMAKE_CURRENT_LIST_DIR}/fileio/PODReader.h
	${CMAKE_CURRENT_LIST_DIR}/Geometry.h
	${CMAKE_CURRENT_LIST_DIR}/Helper.cpp
	${CMAKE_CURRENT_LIST_DIR}/Helper.h
	${CMAKE_CURRENT_LIST_DIR}/IndexedArray.h
	${CMAKE_CURRENT_LIST_DIR}/Model.h
	${CMAKE_CURRENT_LIST_DIR}/model/Animation.cpp
	${CMAKE_CURRENT_LIST_DIR}/model/Animation.h
	${CMAKE_CURRENT_LIST_DIR}/model/Camera.cpp
	${CMAKE_CURRENT_LIST_DIR}/model/Camera.h
	${CMAKE_CURRENT_LIST_DIR}/model/Light.cpp
	${CMAKE_CURRENT_LIST_DIR}/model/Light.h
	${CMAKE_CURRENT_LIST_DIR}/model/Mesh.cpp
	${CMAKE_CURRENT_LIST_DIR}/model/Mesh.h
	${CMAKE_CURRENT_LIST_DIR}/model/Model.cpp
	${CMAKE_CURRENT_LIST_DIR}/PVRAssets.h
	${CMAKE_CURRENT_LIST_DIR}/ShadowVolume.cpp
	${CMAKE_CURRENT_LIST_DIR}/ShadowVolume.h
	${CMAKE_CURRENT_LIST_DIR}/Volume.cpp
	${CMAKE_CURRENT_LIST_DIR}/Volume.h)
	
set(PVRAssets_LINK_LIBS PowerVR_SDK PVRCore)
	
get_directory_property(HAS_PARENT PARENT_DIRECTORY)
if(HAS_PARENT AND USE_PREBUILT_DEPENDENCIES)
add_library(PVRAssets STATIC IMPORTED GLOBAL)

# add the subdirectory after the add_library so that the target exists
add_subdirectory(../.. ${CMAKE_CURRENT_BINARY_DIR}/sdk EXCLUDE_FROM_ALL)

set_target_properties(PVRAssets PROPERTIES
	IMPORTED_LOCATION_DEBUG ${FRAMEWORK_LIB_FOLDER}/${CMAKE_STATIC_LIBRARY_PREFIX}PVRAssets${CMAKE_STATIC_LIBRARY_SUFFIX}
	IMPORTED_LOCATION_RELEASE ${FRAMEWORK_LIB_FOLDER}/${CMAKE_STATIC_LIBRARY_PREFIX}PVRAssets${CMAKE_STATIC_LIBRARY_SUFFIX}
	INTERFACE_LINK_LIBRARIES "${PVRAssets_LINK_LIBS}")
else()
add_library(PVRAssets STATIC EXCLUDE_FROM_ALL ${PVRAssets_SRC})

# add the subdirectory after the add_library so that the target exists
add_subdirectory(../.. ${CMAKE_CURRENT_BINARY_DIR}/sdk EXCLUDE_FROM_ALL)

target_link_libraries(PVRAssets ${PVRAssets_LINK_LIBS})
set_target_properties(PVRAssets PROPERTIES 
	ARCHIVE_OUTPUT_DIRECTORY ${FRAMEWORK_LIB_FOLDER}
	LIBRARY_OUTPUT_DIRECTORY ${FRAMEWORK_LIB_FOLDER})
endif()