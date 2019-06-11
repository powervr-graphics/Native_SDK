cmake_minimum_required(VERSION 3.3)
project(VulkanExampleUI)
if(APPLE)
	message ("Vulkan is not supported on Apple platforms.")
	return()
endif()
add_subdirectory(../../.. ${CMAKE_CURRENT_BINARY_DIR}/sdk)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT VulkanExampleUI)

set(SRC_FILES VulkanExampleUI.cpp)

set(ASSET_FOLDER ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Assets_VulkanExampleUI)

# ASSET FILES (textures, scene files, textures etc) - Packaged with the executable for runtime use
set(ASSET_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/ExampleUI/container-corner.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/ExampleUI/container-filler.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/ExampleUI/container-vertical.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/ExampleUI/container-horizontal.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/ExampleUI/topbar.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/ExampleUI/background.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/ExampleUI/clock-face.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/ExampleUI/clock-face-small.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/ExampleUI/hand.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/ExampleUI/hand-small.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/ExampleUI/battery.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/ExampleUI/internet-web-browser.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/ExampleUI/mail-message-new.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/ExampleUI/network-wireless.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/ExampleUI/office-calendar.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/ExampleUI/weather-sun-cloud.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/ExampleUI/weather-storm.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/ExampleUI/weather-rain.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/ExampleUI/text1.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/ExampleUI/text2.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/ExampleUI/text-weather.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/ExampleUI/text-fri.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/ExampleUI/text-sat.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/ExampleUI/text-sun.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/ExampleUI/text-mon.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/ExampleUI/weather-sun-cloud-big.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/ExampleUI/window-bottom.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/ExampleUI/window-bottomcorner.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/ExampleUI/window-side.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/ExampleUI/window-top.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/ExampleUI/window-topleft.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/ExampleUI/window-topright.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/ExampleUI/loremipsum.pvr
	${CMAKE_CURRENT_SOURCE_DIR}/../../assets/ExampleUI/vertical-bar.pvr
	)
list(APPEND SRC_FILES ${ASSET_FILES})
source_group("assets" FILES ${ASSET_FILES})

# SHADER SOURCE FILES (Vulkan) - Compiled to spir-v at build-time
set(SHADER_NAMES
ColShader.fsh
ColShader.vsh
	)

foreach (SHADER_FILE ${SHADER_NAMES})
	list(APPEND SHADER_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${SHADER_FILE}")
	list(APPEND SHADER_GENERATED_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${SHADER_FILE}.spv")
endforeach()
list(APPEND SRC_FILES ${SHADER_SOURCE_FILES})
list(APPEND SRC_FILES ${SHADER_GENERATED_FILES})
source_group("shaders_source" FILES ${SHADER_SOURCE_FILES})
source_group("shaders_generated" FILES ${SHADER_GENERATED_FILES})

set(RESOURCE_FILES ${ASSET_FILES} ${SHADER_SOURCE_FILES} ${SHADER_GENERATED_FILES})

# Adds Windows resouces.rc, macOS plists etc. For macOS/iOS, also set the opengl dynamic libs in the "frameworks" group
add_platform_specific_resource_files(SRC_FILES RESOURCE_FILES)

# Generate a pre-build step for compiling glsl shaders to spir-v
add_rule_generate_spirv_from_shaders("${SHADER_NAMES}")

# Adds a platform specific executable
add_platform_specific_executable(VulkanExampleUI "${SRC_FILES}" "${RESOURCE_FILES}")

# Copies the assets to our assets folder
add_rule_copy_assets_to_asset_folder("${RESOURCE_FILES}" "${ASSET_FOLDER}")

target_link_libraries(VulkanExampleUI
	PVRShell
	PVRUtilsVk
)