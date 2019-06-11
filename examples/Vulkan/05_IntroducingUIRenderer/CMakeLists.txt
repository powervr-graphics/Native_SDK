cmake_minimum_required(VERSION 3.3)
project(VulkanIntroducingUIRenderer)
if(APPLE)
	message ("Vulkan is not supported on Apple platforms.")
	return()
endif()
add_subdirectory(../../.. ${CMAKE_CURRENT_BINARY_DIR}/sdk)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT VulkanIntroducingUIRenderer)

set(SRC_FILES VulkanIntroducingUIRenderer.cpp)

set(ASSET_FOLDER ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Assets_VulkanIntroducingUIRenderer)

# ASSET FILES (textures, scene files, textures etc) - Packaged with the executable for runtime use
set(ASSET_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/IntroducingUIRenderer/arial_36.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/IntroducingUIRenderer/starjout_60.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/IntroducingUIRenderer/title_36.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/IntroducingUIRenderer/title_46.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/IntroducingUIRenderer/title_56.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/IntroducingUIRenderer/Text.txt
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/IntroducingUIRenderer/titletext.txt
	)
list(APPEND SRC_FILES ${ASSET_FILES})
source_group("assets" FILES ${ASSET_FILES})

set(RESOURCE_FILES ${ASSET_FILES})

# Adds Windows resouces.rc, macOS plists etc. For macOS/iOS, also set the opengl dynamic libs in the "frameworks" group
add_platform_specific_resource_files(SRC_FILES RESOURCE_FILES)

# Adds a platform specific executable
add_platform_specific_executable(VulkanIntroducingUIRenderer "${SRC_FILES}" "${RESOURCE_FILES}")

# Copies the assets to our assets folder
add_rule_copy_assets_to_asset_folder("${RESOURCE_FILES}" "${ASSET_FOLDER}")

target_link_libraries(VulkanIntroducingUIRenderer
	PVRShell
	PVRUtilsVk
)