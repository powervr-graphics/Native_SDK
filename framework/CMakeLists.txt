cmake_minimum_required(VERSION 3.18.0 FATAL_ERROR)
project(PowerVR_Framework)

# Add the default parts of the framework
add_subdirectory(PVRCore)
add_subdirectory(PVRAssets)
add_subdirectory(PVRShell)
add_subdirectory(PVRCamera)

# Add in the APIs requested by the user 
if(${PVR_BUILD_FRAMEWORK_GLES})
	add_subdirectory(PVRUtils/OpenGLES)
endif()

if(${PVR_BUILD_FRAMEWORK_GLES})
	add_subdirectory(PVRUtils/OpenGLSC)
endif()

if(${PVR_BUILD_FRAMEWORK_OPENCL})
	add_subdirectory(PVRUtils/OpenCL)
endif()

# Vulkan is a special case as it requires the Surface extension be specified 
# This is to ensure the correct headers for the surface are included
# Usually this is fixed depending on the operating system
# However Linux can use multiple different types surface 
# As a result, when building the Vulkan framework on linux
# The user is required to pass -DPVR_WINDOW_SYSTEM
if(${PVR_BUILD_FRAMEWORK_VULKAN})

	# Since we know the user is requesting Vulkan framework components 
	# Add in the submodules that are requirements in Vulkan
	add_submodule_to_sdk(${SDK_ROOT_INTERNAL_DIR}/external/VulkanMemoryAllocator)
	add_submodule_to_sdk(${SDK_ROOT_INTERNAL_DIR}/external/glslang)

	add_subdirectory(PVRVk)
	add_subdirectory(PVRUtils/Vulkan)
	add_subdirectory(PVRPfx)
endif()