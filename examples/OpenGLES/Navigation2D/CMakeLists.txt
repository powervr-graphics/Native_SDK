cmake_minimum_required(VERSION 3.3)

project(OpenGLESNavigation2D)

set (OPENGLES_EXAMPLE 1)

#Include common functionality.  (Common.cmake)
# Sets up variables ( ${PROJECT_ARCH}, ${SDK_ROOT},${EXTERNAL_LIB_FOLDER}), sets up some defaults (e.g. CMAKE_BUILD_TYPE),sets up the include folders,
# sets up necessary libraries in EXTRA_LIBS (like dynamic linking, Android libaries, X11/xcb/Wayland etc. for Linux), 
# sets up some compilation flags (e.g. link time code generation, disables some warnings which hit on system files etc.)
include (../../../cmake/Common.cmake)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT OpenGLESNavigation2D)

set (SRC_FILES OpenGLESNavigation2D.cpp ../../common/NavDataProcess.cpp ../../common/NavDataProcess2D.cpp ../../common/NavDataProcess.h)

set (ASSET_FOLDER ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Assets_OpenGLESNavigation2D)

# ASSET FILES (textures, scene files, textures etc) - Packaged with the executable for runtime use
set (ASSET_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/ATM.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/bank.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/bar.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/butcher.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/cafe.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/casino.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/cinema.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/college.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/courthouse.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/dentist.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/doctors.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/embassy.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/fastfood.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/fire.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/florist.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/font.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/hairdresser.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/hospital.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/library.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/optician.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/other.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/parking.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/petrol.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/pharmacy.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/police.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/postbox.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/postoffice.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/pub.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/restaurant.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/Road.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/shop.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/theatre.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/toilets.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/university.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/vets.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/worship.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/Navigation2D/map.osm
	)
list (APPEND SRC_FILES ${ASSET_FILES})
source_group("assets" FILES ${ASSET_FILES})

# TEXTUAL SHADER FILES (OpenGL ES) - Packaged with the executable for runtime use
set (SHADER_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/AA_VertShader.vsh
	${CMAKE_CURRENT_SOURCE_DIR}/AA_FragShader.fsh
	${CMAKE_CURRENT_SOURCE_DIR}/VertShader.vsh
	${CMAKE_CURRENT_SOURCE_DIR}/FragShader.fsh
	)
list (APPEND SRC_FILES ${SHADER_FILES})
source_group("shaders" FILES ${SHADER_FILES})

set (RESOURCE_FILES ${ASSET_FILES} ${SHADER_FILES} ${SHADER_GENERATED_FILES})

# Adds Windows resouces.rc, macOS plists etc. For macOS/iOS, also set the opengl dynamic libs in the "frameworks" group
add_platform_specific_resource_files("SRC_FILES" "RESOURCE_FILES")

if (WIN32)
	add_executable( OpenGLESNavigation2D WIN32 ${SRC_FILES})
elseif (ANDROID)
	add_library( OpenGLESNavigation2D SHARED ${SRC_FILES})
elseif (APPLE)
	add_executable(OpenGLESNavigation2D MACOSX_BUNDLE ${SRC_FILES} ${RESOURCE_FILES} ${FRAMEWORK_FILES})
	set_target_properties(OpenGLESNavigation2D PROPERTIES MACOSX_BUNDLE_INFO_PLIST  "${INFO_PLIST_FILE}")
	set_target_properties(OpenGLESNavigation2D PROPERTIES RESOURCE "${RESOURCE_FILES}")
elseif (UNIX OR QNX)
	add_executable( OpenGLESNavigation2D ${SRC_FILES})
endif()

add_rule_copy_assets_to_asset_folder("${RESOURCE_FILES}" "${ASSET_FOLDER}")

# Add the Framework subprojects.
if (NOT ANDROID) # Android builds the Framework libraries by using Gradle to kick CMake (this is an Android thing...)
	add_subdirectory_if_not_already_included(PVRCore ${SDK_ROOT}/framework/PVRCore ${FRAMEWORK_CMAKE_FILES_FOLDER}/PVRCore)
	add_subdirectory_if_not_already_included(PVRAssets ${SDK_ROOT}/framework/PVRAssets ${FRAMEWORK_CMAKE_FILES_FOLDER}/PVRAssets)
	add_subdirectory_if_not_already_included(PVRShell ${SDK_ROOT}/framework/PVRShell ${FRAMEWORK_CMAKE_FILES_FOLDER}/PVRShell)
	add_subdirectory_if_not_already_included(PVRUtilsGles ${SDK_ROOT}/framework/PVRUtils/OpenGLES ${FRAMEWORK_CMAKE_FILES_FOLDER}/PVRUtilsGles)

	add_dependencies(OpenGLESNavigation2D PVRCore  PVRAssets  PVRShell  PVRUtilsGles) #... We should still add the dependencies, build or not.
endif()

target_link_libraries(OpenGLESNavigation2D
${FRAMEWORK_LIB_FOLDER}/${CMAKE_STATIC_LIBRARY_PREFIX}PVRUtilsGles${CMAKE_STATIC_LIBRARY_SUFFIX}
${FRAMEWORK_LIB_FOLDER}/${CMAKE_STATIC_LIBRARY_PREFIX}PVRShell${CMAKE_STATIC_LIBRARY_SUFFIX}
${FRAMEWORK_LIB_FOLDER}/${CMAKE_STATIC_LIBRARY_PREFIX}PVRAssets${CMAKE_STATIC_LIBRARY_SUFFIX}
${FRAMEWORK_LIB_FOLDER}/${CMAKE_STATIC_LIBRARY_PREFIX}PVRCore${CMAKE_STATIC_LIBRARY_SUFFIX}
${EXTRA_LIBS})

target_compile_definitions(OpenGLESNavigation2D PUBLIC $<$<CONFIG:Debug>:DEBUG=1> $<$<NOT:$<CONFIG:Debug>>:RELEASE=1> ) #Defines DEBUG=1 or RELEASE=1

