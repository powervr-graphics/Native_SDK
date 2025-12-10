# FindPVRUtilsVk.cmake
#
# Finds the PVRUtilsVk library and its dependencies
#
# This will define the following imported targets
#     PVRCore
#     PVRAssets
#     PVRVk
#     VulkanMemoryAllocator
#     glslang
#     SPIRV
# 	  MachineIndependent
#	  GenericCodeGen

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})
include(CMakeFindDependencyMacro)

# Include helper for Android paths
include("${CMAKE_CURRENT_LIST_DIR}/../utilities/android_utils.cmake")

if(NOT TARGET PVRCore)
	find_dependency(PVRCore REQUIRED MODULE)
endif()

if(NOT TARGET PVRAssets)
	find_dependency(PVRAssets REQUIRED MODULE)
endif()

if(NOT TARGET PVRVk)
	find_dependency(PVRVk REQUIRED MODULE)
endif()

if(NOT TARGET VulkanMemoryAllocator)
	find_dependency(VulkanMemoryAllocator REQUIRED MODULE)
endif()

if(NOT TARGET glslang)
	find_dependency(glslang REQUIRED MODULE)
endif()

if(NOT TARGET SPIRV)
	find_dependency(SPIRV REQUIRED MODULE)
endif()

if(NOT TARGET GenericCodeGen)
	find_dependency(GenericCodeGen REQUIRED MODULE)
endif()

if(NOT TARGET MachineIndependent)
	find_dependency(MachineIndependent REQUIRED MODULE)
endif()

if(PVR_PREBUILT_DEPENDENCIES)
	if(ANDROID)
		# Allow finding packages in the host file system
		set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)

        pvr_find_android_build_path(PVRUtilsVk_DIR "${CMAKE_CURRENT_LIST_DIR}/../../framework/PVRUtils/Vulkan/build-android" "PVRUtilsVk")
	endif()
endif()

if(NOT TARGET PVRUtilsVk)
	# Try to find the package configuration
	find_package(PVRUtilsVk CONFIG QUIET)
	
    if(PVRUtilsVk_FOUND)
        message(STATUS "PVRUtilsVk: Package configuration found.")
        # Ensure the framework source directory is in the include path.
        get_filename_component(PVR_FRAMEWORK_DIR "${CMAKE_CURRENT_LIST_DIR}/../../framework" ABSOLUTE)
        get_filename_component(PVR_SDK_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/../../include" ABSOLUTE)
        
        set_property(TARGET PVRUtilsVk APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${PVR_FRAMEWORK_DIR}" "${PVR_SDK_INCLUDE_DIR}")
    else()
        message(STATUS "PVRUtilsVk: Prebuilt package not found. Attempting to build from source...")
        set(PVRUtilsVk_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../../framework/PVRUtils/Vulkan")
        if(EXISTS "${PVRUtilsVk_SOURCE_DIR}/CMakeLists.txt")
            add_subdirectory("${PVRUtilsVk_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/framework/PVRUtils/Vulkan")
        else()
            message(FATAL_ERROR "PVRUtilsVk: Could not find prebuilt package AND could not find source at ${PVRUtilsVk_SOURCE_DIR}")
        endif()
    endif()
endif()
