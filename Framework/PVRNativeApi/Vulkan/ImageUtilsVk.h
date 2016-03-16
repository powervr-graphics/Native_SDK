/*!*********************************************************************************************************************
\file         PVRNativeApi\Vulkan\BufferUtilsVk.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        contain functions for creating Vulkan Image object.
***********************************************************************************************************************/
#pragma once
#include "VulkanBindings.h"
#include "PVRCore/Defines.h"
#include "PVRAssets/Texture/PixelFormat.h"
namespace pvr {
namespace apiutils {
namespace vulkan {

/*!******************************************************************************************************************************
\brief allocate image's device memory
\return Return true if the allocation success
\param[in] device Device used for allocation
\param[in] deviceMemProperty Physical device memory property
\param[in] allocMemProperty memory allocation flag (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT etc )
\param[in] image The image the memory to be allocated for
\param[inout] outMemRequirements Return memory requirements if not NULL
********************************************************************************************************************************/
bool allocateImageDeviceMemory(VkDevice device, VkPhysicalDeviceMemoryProperties& deviceMemProperty,
                               VkMemoryPropertyFlagBits allocMemProperty, native::HTexture_& image,
                               VkMemoryRequirements* outMemRequirements);
}
}
}
