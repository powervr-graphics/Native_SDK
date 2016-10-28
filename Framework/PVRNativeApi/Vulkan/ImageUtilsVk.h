/*!*********************************************************************************************************************
\file         PVRNativeApi/Vulkan/ImageUtilsVk.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        contain functions for creating Vulkan Image object.
***********************************************************************************************************************/
#pragma once
#include "VulkanBindings.h"
#include "PVRCore/Defines.h"
#include "PVRAssets/PixelFormat.h"
namespace pvr {
namespace utils {
namespace vulkan {

/*!********************************************************************************************************************
\brief The ImageUpdateParam struct.
***********************************************************************************************************************/
struct ImageUpdateParam
{
	// 1D/Array texture and common for rest
	int32   offsetX;  //!< Valid for all
	uint32    width;    //!< Valid for all
	uint32    arrayIndex; //!< Valid for 1D,2D and Cube texture updates
	uint32    mipLevel; //!< Valid for all
	const void* data;   //!< Valid for all
	uint32    dataSize; //!< Valid for all


	// 2D/ Array texture only
	int32   offsetY;  //!< Valid for 2D, 3D and Cube texture updates
	uint32    height;   //!< Valid for 2D, 3D and Cube texture updates

	// cube/ Array Map only. Derive all states above
	uint32    cubeFace; //!< Valid for Cube texture updates only

	// 3D texture Only. Derive all states above Except arrayIndex
	int32   offsetZ;  //!< Valid for 3D texture updates only
	uint32    depth;    //!< Valid for texture updates only

	ImageUpdateParam() : offsetX(0), width(1), mipLevel(0), data(0), dataSize(0), offsetY(0), height(1),
		cubeFace(0), offsetZ(0), depth(1) {}
};

/*!
   \brief Create image and memory
   \param context
   \param dimension
   \param arrayLayers
   \param sampleCount
   \param numMipLevels
   \param tilingOptimal
   \param isCubeMap
   \param imageType
   \param format
   \param imageUsageFlags
   \param newLayout
   \param memPropertyFlags
   \param outTexture
   \return Return true on success
 */
bool createImageAndMemory(IPlatformContext& context, const types::Extent3D& dimension, uint32 arrayLayers,
                          VkSampleCountFlagBits sampleCount, uint32 numMipLevels, bool tilingOptimal, bool isCubeMap,
                          VkImageType imageType, VkFormat format, VkImageUsageFlags imageUsageFlags, VkImageLayout  newLayout,
                          VkMemoryPropertyFlagBits memPropertyFlags, native::HTexture_& outTexture);


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

/*!
   \brief Set image layout
   \param cmd Recording commandbuffer for image layout transition command.
   \param oldLayout
   \param newLayout
   \param image
   \param aspectFlags
   \param baseMipLevel
   \param numMipLevels
   \param baseArrayLayer
   \param numArrayLayers
 */
void setImageLayout(VkCommandBuffer& cmd, VkImageLayout oldLayout, VkImageLayout newLayout,
                    VkImage image, VkImageAspectFlags aspectFlags, uint32 baseMipLevel, uint32 numMipLevels,
                    uint32 baseArrayLayer, uint32 numArrayLayers);

/*!*
\brief Utility function to update an image's data
\return Return true if success
\param[in] context The contenxt of the image
\param[in] updateParams This object is a c-style array of areas and the data to upload.
\param[in] numUpdateParams The number of ImageUpdateParam objects in \p updateParams
\param[in] numArraySlice The number of array slices of the image
\param[in] srcFormat The format of the image
\param isCubeMap Is the image a cubemap
\param image The image to update
*/
bool updateImage(IPlatformContext& context, ImageUpdateParam* updateParams, uint32 numUpdateParams,
                 uint32 numArraySlice, VkFormat srcFormat, bool isCubeMap, VkImage image);
}
}
}
