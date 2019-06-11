cmake_minimum_required(VERSION 3.3)
project (PVRUtilsVk)

set(PVRUtilsVk_SRC
    ${CMAKE_CURRENT_LIST_DIR}/../ArialBoldFont.h
    ${CMAKE_CURRENT_LIST_DIR}/../MultiObject.h
    ${CMAKE_CURRENT_LIST_DIR}/../PowerVRLogo.h
	${CMAKE_CURRENT_LIST_DIR}/../PBRUtils.h
    ${CMAKE_CURRENT_LIST_DIR}/../PVRUtilsTypes.h
    ${CMAKE_CURRENT_LIST_DIR}/../PVRUtilsVk.h
    ${CMAKE_CURRENT_LIST_DIR}/../StructuredMemory.h
	${CMAKE_CURRENT_LIST_DIR}/AsynchronousVk.h
	${CMAKE_CURRENT_LIST_DIR}/ConvertToPVRVkTypes.h
	${CMAKE_CURRENT_LIST_DIR}/HelperVk.cpp
	${CMAKE_CURRENT_LIST_DIR}/HelperVk.h
	${CMAKE_CURRENT_LIST_DIR}/MemoryAllocator.cpp
	${CMAKE_CURRENT_LIST_DIR}/MemoryAllocator.h
	${CMAKE_CURRENT_LIST_DIR}/PBRUtilsVk.cpp
	${CMAKE_CURRENT_LIST_DIR}/PBRUtilsVk.h
	${CMAKE_CURRENT_LIST_DIR}/PBRUtilsVertShader.h
	${CMAKE_CURRENT_LIST_DIR}/PBRUtilsIrradianceFragShader.h
	${CMAKE_CURRENT_LIST_DIR}/PBRUtilsPrefilteredFragShader.h
	${CMAKE_CURRENT_LIST_DIR}/ShaderUtilsVk.cpp
	${CMAKE_CURRENT_LIST_DIR}/ShaderUtilsVk.h
	${CMAKE_CURRENT_LIST_DIR}/SpriteVk.cpp
	${CMAKE_CURRENT_LIST_DIR}/SpriteVk.h
	${CMAKE_CURRENT_LIST_DIR}/UIRendererFragShader.h
	${CMAKE_CURRENT_LIST_DIR}/UIRendererVertShader.h
	${CMAKE_CURRENT_LIST_DIR}/UIRendererVk.cpp
	${CMAKE_CURRENT_LIST_DIR}/UIRendererVk.h)
	
set(PVRUtilsVk_LINK_LIBS PowerVR_SDK PVRAssets PVRCore PVRVk glslang SPIRV OGLCompiler OSDependent VulkanMemoryAllocator)
	
get_directory_property(HAS_PARENT PARENT_DIRECTORY)
if(HAS_PARENT AND USE_PREBUILT_DEPENDENCIES)
add_library(PVRUtilsVk STATIC IMPORTED GLOBAL)

# add the subdirectory after the add_library so that the target exists
add_subdirectory(../../.. ${CMAKE_CURRENT_BINARY_DIR}/sdk EXCLUDE_FROM_ALL)

set_target_properties(PVRUtilsVk PROPERTIES
	IMPORTED_LOCATION_DEBUG ${FRAMEWORK_LIB_FOLDER}/${CMAKE_STATIC_LIBRARY_PREFIX}PVRUtilsVk${CMAKE_STATIC_LIBRARY_SUFFIX}
	IMPORTED_LOCATION_RELEASE ${FRAMEWORK_LIB_FOLDER}/${CMAKE_STATIC_LIBRARY_PREFIX}PVRUtilsVk${CMAKE_STATIC_LIBRARY_SUFFIX}
	INTERFACE_LINK_LIBRARIES "${PVRUtilsVk_LINK_LIBS}")
else()
add_library(PVRUtilsVk STATIC EXCLUDE_FROM_ALL ${PVRUtilsVk_SRC})

# add the subdirectory after the add_library so that the target exists
add_subdirectory(../../.. ${CMAKE_CURRENT_BINARY_DIR}/sdk EXCLUDE_FROM_ALL)

target_link_libraries(PVRUtilsVk ${PVRUtilsVk_LINK_LIBS})
set_target_properties(PVRUtilsVk PROPERTIES 
	ARCHIVE_OUTPUT_DIRECTORY ${FRAMEWORK_LIB_FOLDER}
	LIBRARY_OUTPUT_DIRECTORY ${FRAMEWORK_LIB_FOLDER})
endif()