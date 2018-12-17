cmake_minimum_required(VERSION 3.3)

project(VulkanNavigation2D)

set (VULKAN_EXAMPLE 1)

#Include common functionality.  (Common.cmake)
# Sets up variables ( ${PROJECT_ARCH}, ${SDK_ROOT},${EXTERNAL_LIB_FOLDER}), sets up some defaults (e.g. CMAKE_BUILD_TYPE),sets up the include folders,
# sets up necessary libraries in EXTRA_LIBS (like dynamic linking, Android libaries, X11/xcb/Wayland etc. for Linux), 
# sets up some compilation flags (e.g. link time code generation, disables some warnings which hit on system files etc.)
include (../../../cmake/Common.cmake)

if(APPLE)
	message ("Vulkan is not supported on Apple platforms.")
	return()
endif()

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT VulkanNavigation2D)

set (SRC_FILES VulkanNavigation2D.cpp ../../common/NavDataProcess.cpp ../../common/NavDataProcess2D.cpp ../../common/NavDataProcess.h)

set (ASSET_FOLDER ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Assets_VulkanNavigation2D)

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

# SHADER SOURCE FILES (Vulkan) - Compiled to spir-v at build-time
set (SHADER_NAMES
AA_VertShader.vsh
AA_FragShader.fsh
VertShader.vsh
FragShader.fsh
	)

foreach (SHADER_FILE ${SHADER_NAMES})
	list(APPEND SHADER_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${SHADER_FILE}")
	list(APPEND SHADER_GENERATED_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${SHADER_FILE}.spv")
endforeach()
list (APPEND SRC_FILES ${SHADER_SOURCE_FILES})
list (APPEND SRC_FILES ${SHADER_GENERATED_FILES})
source_group("shaders_source" FILES ${SHADER_SOURCE_FILES})
source_group("shaders_generated" FILES ${SHADER_GENERATED_FILES})

set (RESOURCE_FILES ${ASSET_FILES} ${SHADER_FILES} ${SHADER_GENERATED_FILES})

# Adds Windows resouces.rc, macOS plists etc. For macOS/iOS, also set the opengl dynamic libs in the "frameworks" group
add_platform_specific_resource_files("SRC_FILES" "RESOURCE_FILES")

#GENERATE A PRE-BUILD STEP FOR COMPILING GLSL TO SPIRV
add_rule_generate_spirv_from_shaders("${SHADER_NAMES}")

if (WIN32)
	add_executable( VulkanNavigation2D WIN32 ${SRC_FILES})
elseif (ANDROID)
	add_library( VulkanNavigation2D SHARED ${SRC_FILES})

elseif (UNIX OR QNX)
	add_executable( VulkanNavigation2D ${SRC_FILES})
endif()

add_rule_copy_assets_to_asset_folder("${RESOURCE_FILES}" "${ASSET_FOLDER}")

# Add the Framework subprojects.
if (NOT ANDROID) # Android builds the Framework libraries by using Gradle to kick CMake (this is an Android thing...)
	add_subdirectory_if_not_already_included(PVRCore ${SDK_ROOT}/framework/PVRCore ${FRAMEWORK_CMAKE_FILES_FOLDER}/PVRCore)
	add_subdirectory_if_not_already_included(PVRAssets ${SDK_ROOT}/framework/PVRAssets ${FRAMEWORK_CMAKE_FILES_FOLDER}/PVRAssets)
	add_subdirectory_if_not_already_included(PVRVk ${SDK_ROOT}/framework/PVRVk ${FRAMEWORK_CMAKE_FILES_FOLDER}/PVRVk)
	add_subdirectory_if_not_already_included(PVRShell ${SDK_ROOT}/framework/PVRShell ${FRAMEWORK_CMAKE_FILES_FOLDER}/PVRShell)
	add_subdirectory_if_not_already_included(PVRUtilsVk ${SDK_ROOT}/framework/PVRUtils/Vulkan ${FRAMEWORK_CMAKE_FILES_FOLDER}/PVRUtilsVk)

	add_dependencies(VulkanNavigation2D PVRCore  PVRAssets  PVRVk  PVRShell  PVRUtilsVk) #... We should still add the dependencies, build or not.
endif()

target_link_libraries(VulkanNavigation2D
${FRAMEWORK_LIB_FOLDER}/${CMAKE_STATIC_LIBRARY_PREFIX}PVRUtilsVk${CMAKE_STATIC_LIBRARY_SUFFIX}
${FRAMEWORK_LIB_FOLDER}/${CMAKE_STATIC_LIBRARY_PREFIX}PVRShell${CMAKE_STATIC_LIBRARY_SUFFIX}
${FRAMEWORK_LIB_FOLDER}/${CMAKE_STATIC_LIBRARY_PREFIX}PVRVk${CMAKE_STATIC_LIBRARY_SUFFIX}
${FRAMEWORK_LIB_FOLDER}/${CMAKE_STATIC_LIBRARY_PREFIX}PVRAssets${CMAKE_STATIC_LIBRARY_SUFFIX}
${FRAMEWORK_LIB_FOLDER}/${CMAKE_STATIC_LIBRARY_PREFIX}PVRCore${CMAKE_STATIC_LIBRARY_SUFFIX}
${EXTRA_LIBS})

target_compile_definitions(VulkanNavigation2D PUBLIC $<$<CONFIG:Debug>:DEBUG=1> $<$<NOT:$<CONFIG:Debug>>:RELEASE=1> ) #Defines DEBUG=1 or RELEASE=1

