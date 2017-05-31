/*!
\brief contain functions for creating Vulkan Buffer object.
\file PVRNativeApi/Vulkan/BufferUtilsVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#pragma once
#include "PVRCore/Interfaces/ForwardDecApiObjects.h"
#include "PVRNativeApi/Vulkan/NativeObjectsVk.h"
#include "PVRNativeApi/Vulkan/VulkanBindings.h"

namespace pvr {
namespace utils {
namespace vulkan {
/// <summary>create buffer</summary>
/// <param name="context">PlatformContext used for allocation</param>
/// <param name="usage">Buffer binding use</param>
/// <param name="size">Buffer size</param>
/// <param name="memHostVisible">Allow Buffer memory to be host visible for map and un-map operation</param>
/// <param name="outBuffer">The buffer used for memory allocation</param>
/// <returns>Return true if success</returns>
bool createBuffer(IPlatformContext& context, types::BufferBindingUse usage,
                  uint32 size, bool memHostVisible, native::HBuffer_& outBuffer);

<<<<<<< HEAD
/*!******************************************************************************************************************************
\brief Allocate memory for a buffer.
\return Return true if the allocation success
\param[in] device The device used for allocation
\param[in] deviceMemProperty The memory properties of the device (i.e. as queried from the device)
\param[in] allocMemProperty The memory properties that are required for this memory allocation (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT etc )
\param[inout] buffer The buffer from which requirements will be queried from, and to which the memory will be bound.
\param[out] outMemRequirements If not NULL, the memory requirements of this allocation will be written here.
********************************************************************************************************************************/
=======
/// <summary>Allocate memory for a buffer.</summary>
/// <param name="device">The device used for allocation</param>
/// <param name="deviceMemProperty">The memory properties of the device (i.e. as queried from the device)</param>
/// <param name="allocMemProperty">The memory properties that are required for this memory allocation
/// (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT etc )</param>
/// <param name="buffer">The buffer from which requirements will be queried from, and to which the memory will be
/// bound.</param>
/// <param name="outMemRequirements">If not NULL, the memory requirements of this allocation will be written
/// here.</param>
/// <returns>Return true if the allocation success</returns>
>>>>>>> 1776432f... 4.3
bool allocateBufferDeviceMemory(VkDevice device, const VkPhysicalDeviceMemoryProperties& deviceMemProperty,
                                VkMemoryPropertyFlagBits allocMemProperty, native::HBuffer_& buffer,
                                VkMemoryRequirements* outMemRequirements);

<<<<<<< HEAD
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
=======
/// <summary>Create a Buffer together with its underlying memory</summary>
/// <param name="device">The device used for allocation</param>
/// <param name="deviceMemProperty">The memory properties of the device (i.e. as queried from the device)</param>
/// <param name="allocMemProperty">The memory properties that are required for this memory allocation
/// (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT etc )</param>
/// <param name="usage">All usages that the buffer must be valid for</param>
/// <param name="size">The size of the buffer</param>
/// <param name="outBuffer">The buffer to be created</param>
/// <param name="outMemRequirements">If not NULL, the memory requirements of this allocation will be written
/// here.</param>
/// <returns>Return true if succceed</returns>
>>>>>>> 1776432f... 4.3
bool createBufferAndMemory(VkDevice device, const VkPhysicalDeviceMemoryProperties& deviceMemProperty,
                           VkMemoryPropertyFlagBits allocMemProperty, types::BufferBindingUse usage,
                           pvr::uint32 size, native::HBuffer_& outBuffer, VkMemoryRequirements* outMemRequirements);

}
}
}

//!\endcond