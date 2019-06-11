cmake_minimum_required(VERSION 3.3)
project (PVRUtilsGles)

set(PVRUtilsGles_SRC 
    ${CMAKE_CURRENT_LIST_DIR}/../ArialBoldFont.h
    ${CMAKE_CURRENT_LIST_DIR}/../MultiObject.h
    ${CMAKE_CURRENT_LIST_DIR}/../PowerVRLogo.h
	${CMAKE_CURRENT_LIST_DIR}/../PBRUtils.h
    ${CMAKE_CURRENT_LIST_DIR}/../PVRUtilsGles.h
    ${CMAKE_CURRENT_LIST_DIR}/../PVRUtilsTypes.h
    ${CMAKE_CURRENT_LIST_DIR}/../StructuredMemory.h
	${CMAKE_CURRENT_LIST_DIR}/../OpenCL/OpenCLUtils.h
	${CMAKE_CURRENT_LIST_DIR}/BindingsGles.h
	${CMAKE_CURRENT_LIST_DIR}/ConvertToGlesTypes.cpp
	${CMAKE_CURRENT_LIST_DIR}/ConvertToGlesTypes.h
	${CMAKE_CURRENT_LIST_DIR}/ErrorsGles.cpp
	${CMAKE_CURRENT_LIST_DIR}/ErrorsGles.h
	${CMAKE_CURRENT_LIST_DIR}/HelperGles.h
	${CMAKE_CURRENT_LIST_DIR}/ModelGles.cpp
	${CMAKE_CURRENT_LIST_DIR}/ModelGles.h
	${CMAKE_CURRENT_LIST_DIR}/PBRUtilsGles.cpp
	${CMAKE_CURRENT_LIST_DIR}/PBRUtilsGles.h
	${CMAKE_CURRENT_LIST_DIR}/ShaderUtilsGles.cpp
	${CMAKE_CURRENT_LIST_DIR}/ShaderUtilsGles.h
	${CMAKE_CURRENT_LIST_DIR}/SpriteGles.cpp
	${CMAKE_CURRENT_LIST_DIR}/SpriteGles.h
	${CMAKE_CURRENT_LIST_DIR}/TextureUtilsGles.cpp
	${CMAKE_CURRENT_LIST_DIR}/TextureUtilsGles.h
	${CMAKE_CURRENT_LIST_DIR}/UIRendererGles.cpp
	${CMAKE_CURRENT_LIST_DIR}/UIRendererGles.h
	${CMAKE_CURRENT_LIST_DIR}/UIRendererShaders_ES.h)

if (IOS)
	list(APPEND PVRUtilsGles_SRC
		${CMAKE_CURRENT_LIST_DIR}/../EAGL/EaglPlatformContext.mm
		${CMAKE_CURRENT_LIST_DIR}/../EAGL/EaglPlatformHandles.h)
else()
	list(APPEND PVRUtilsGles_SRC
		${CMAKE_CURRENT_LIST_DIR}/../EGL/EglPlatformContext.cpp
		${CMAKE_CURRENT_LIST_DIR}/../EGL/EglPlatformContext.h
		${CMAKE_CURRENT_LIST_DIR}/../EGL/EglPlatformHandles.h)
endif()

set(PVRUtilsGles_LINK_LIBS PowerVR_SDK PVRAssets PVRCore OpenGLESPlatformInterface)

get_directory_property(HAS_PARENT PARENT_DIRECTORY)
if(HAS_PARENT AND USE_PREBUILT_DEPENDENCIES)
add_library(PVRUtilsGles STATIC IMPORTED GLOBAL)

# add the subdirectory after the add_library so that the target exists
add_subdirectory(../../.. ${CMAKE_CURRENT_BINARY_DIR}/sdk EXCLUDE_FROM_ALL)

set_target_properties(PVRUtilsGles PROPERTIES
	IMPORTED_LOCATION_DEBUG ${FRAMEWORK_LIB_FOLDER}/${CMAKE_STATIC_LIBRARY_PREFIX}PVRUtilsGles${CMAKE_STATIC_LIBRARY_SUFFIX}
	IMPORTED_LOCATION_RELEASE ${FRAMEWORK_LIB_FOLDER}/${CMAKE_STATIC_LIBRARY_PREFIX}PVRUtilsGles${CMAKE_STATIC_LIBRARY_SUFFIX}
	INTERFACE_LINK_LIBRARIES "${PVRUtilsGles_LINK_LIBS}")
else()
add_library(PVRUtilsGles STATIC EXCLUDE_FROM_ALL ${PVRUtilsGles_SRC})

# add the subdirectory after the add_library so that the target exists
add_subdirectory(../../.. ${CMAKE_CURRENT_BINARY_DIR}/sdk EXCLUDE_FROM_ALL)

target_link_libraries(PVRUtilsGles ${PVRUtilsGles_LINK_LIBS})
set_target_properties(PVRUtilsGles PROPERTIES 
	ARCHIVE_OUTPUT_DIRECTORY ${FRAMEWORK_LIB_FOLDER}
	LIBRARY_OUTPUT_DIRECTORY ${FRAMEWORK_LIB_FOLDER})
endif()