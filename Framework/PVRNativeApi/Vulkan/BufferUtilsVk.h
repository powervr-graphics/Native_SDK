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
namespace utils {
namespace vulkan {

/*!******************************************************************************************************************************
\brief Allocate memory for a buffer.
\return Return true if the allocation success
\param[in] device The device used for allocation
\param[in] deviceMemProperty The memory properties of the device (i.e. as queried from the device)
\param[in] allocMemProperty The memory properties that are required for this memory allocation (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT etc )
\param[inout] buffer The buffer from which requirements will be queried from, and to which the memory will be bound.
\param[out] outMemRequirements If not NULL, the memory requirements of this allocation will be written here.
********************************************************************************************************************************/
bool allocateBufferDeviceMemory(VkDevice device, const VkPhysicalDeviceMemoryProperties& deviceMemProperty,
                                VkMemoryPropertyFlagBits allocMemProperty, native::HBuffer_& buffer,
                                VkMemoryRequirements* outMemRequirements);

/*!******************************************************************************************************************************
\brief Create a Buffer together with its underlying memory
\return Return true if succceed
\param[in] device The device used for allocation
\param[in] deviceMemProperty The memory properties of the device (i.e. as queried from the device)
\param[in] allocMemProperty The memory properties that are required for this memory allocation (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT etc )
\param[in] usage All usages that the buffer must be valid for
\param[in] size The size of the buffer
\param[in] outBuffer The buffer to be created
\param[out] outMemRequirements If not NULL, the memory requirements of this allocation will be written here.
********************************************************************************************************************************/
bool createBufferAndMemory(VkDevice device, const VkPhysicalDeviceMemoryProperties& deviceMemProperty,
                           VkMemoryPropertyFlagBits allocMemProperty, types::BufferBindingUse usage,
                           pvr::uint32 size, native::HBuffer_& outBuffer, VkMemoryRequirements* outMemRequirements);

}
}
}
