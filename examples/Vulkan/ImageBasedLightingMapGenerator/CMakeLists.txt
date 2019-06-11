cmake_minimum_required(VERSION 3.3)
project(VulkanIBLMapsGenerator)
if(APPLE)
	message ("Vulkan is not supported on Apple platforms.")
	return()
endif()
add_subdirectory(../../.. ${CMAKE_CURRENT_BINARY_DIR}/sdk)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT VulkanIBLMapsGenerator)

set(SRC_FILES VulkanIBLMapsGenerator.cpp)

# Adds Windows resouces.rc, macOS plists etc. For macOS/iOS, also set the opengl dynamic libs in the "frameworks" group
add_platform_specific_resource_files(SRC_FILES "")

# Adds a platform specific executable
add_executable(VulkanIBLMapsGenerator ${SRC_FILES})

target_link_libraries(VulkanIBLMapsGenerator PVRUtilsVk)
