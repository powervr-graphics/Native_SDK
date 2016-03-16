/*!*********************************************************************************************************************
\file         PVRNativeApi\Vulkan\BufferUtilsVk.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        contain functions for creating Vulkan Buffer object.
***********************************************************************************************************************/
#pragma once
#include "PVRCore/ForwardDecApiObjects.h"
#include "PVRNativeApi/Vulkan/VulkanBindings.h"
namespace pvr {
namespace apiUtils {
namespace vulkan {

/*!******************************************************************************************************************************
\brief allocate buffer's device memory
\return Return true if the allocation success
\param[in] device Device used for allocation
\param[in] outBuffer The buffer used for memory allocation
\param[inout] outMemRequirements
********************************************************************************************************************************/
bool allocateBufferDeviceMemory(VkDevice device, VkPhysicalDeviceMemoryProperties& deviceMemProperty,
                                VkMemoryPropertyFlagBits allocMemProperty, native::HBuffer_& buffer,
                                VkMemoryRequirements* outMemRequirements);

/*!******************************************************************************************************************************
\brief create Buffer
\return Return true if succceed
\param[in] device Device used for allocation
\param[in] deviceMemProperty Physical device memory property
\param[in] allocMemProperty memory allocation flag (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT etc )
\param[in] usage Buffer usage
\param[in] size Buffer size
\param[in] outBuffer The buffer to be created
\param[inout] outMemRequirements Return memory requirements if not NULL
********************************************************************************************************************************/
bool createBuffer(VkDevice device, VkPhysicalDeviceMemoryProperties& deviceMemProperty,
                  VkMemoryPropertyFlagBits allocMemProperty, types::BufferBindingUse::Bits usage,
                  pvr::uint32 size, native::HBuffer_& outBuffer, VkMemoryRequirements* outMemRequirements);

}
}
}
