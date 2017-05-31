<<<<<<< HEAD
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
=======
/*!
\brief Functions for creating Vulkan Image objects.
\file PVRNativeApi/Vulkan/ImageUtilsVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#pragma once
#include "PVRNativeApi/Vulkan/NativeObjectsVk.h"
#include "PVRNativeApi/Vulkan/VulkanBindings.h"
#include "PVRCore/Interfaces/IPlatformContext.h"
#include "PVRCore/Interfaces/ForwardDecApiObjects.h"
#include "PVRCore/Texture.h"
#include "PVRCore/Threading.h"

>>>>>>> 1776432f... 4.3
namespace pvr {
namespace utils {
namespace vulkan {

<<<<<<< HEAD
/*!********************************************************************************************************************
\brief The ImageUpdateParam struct.
***********************************************************************************************************************/
=======
/// <summary>The ImageUpdateParam struct.</summary>
>>>>>>> 1776432f... 4.3
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

<<<<<<< HEAD
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
=======
VkImageAspectFlags inferAspectFromFormat(VkFormat format);
VkImageAspectFlags inferAspectFromFormat(PixelFormat format);

/// <summary>Create image and memory. Final layout is VK_IMAGE_LAYOUT_PREINITIALIZED</summary>
/// <param name="context"></param>
/// <param name="dimension"></param>
/// <param name="arrayLayers"></param>
/// <param name="sampleCount"></param>
/// <param name="numMipLevels"></param>
/// <param name="isCubeMap"></param>
/// <param name="imageType"></param>
/// <param name="format"></param>
/// <param name="imageUsageFlags"></param>
/// <param name="newLayout"></param>
/// <param name="memPropertyFlags"></param>
/// <param name="outTexture"></param>
/// <returns>Return true on success</returns>
bool createImageAndMemory(VkDevice device, VkPhysicalDeviceMemoryProperties memprops,
                          const types::Extent3D& dimension, uint32 arrayLayer,
                          VkSampleCountFlagBits sampleCount, uint32 numMipLevels, bool isCubeMap,
                          VkImageType imageType, VkFormat format, VkImageUsageFlags imageUsageFlags,
                          VkMemoryPropertyFlagBits memPropertyFlags,
                          VkImage& outImage, VkDeviceMemory& outMemory);


bool createImageAndMemory(
  platform::NativePlatformHandles_& handles,
  const types::Extent3D& dimension, uint32 arrayLayer,
  VkSampleCountFlagBits sampleCount, uint32 numMipLevels, bool isCubeMap,
  VkImageType imageType, VkFormat format, VkImageUsageFlags imageUsageFlags,
  VkMemoryPropertyFlagBits memPropertyFlags, native::HTexture_& outTexture);

/// <summary>allocate image's device memory</summary>
/// <param name="device">Device used for allocation</param>
/// <param name="deviceMemProperty">Physical device memory property</param>
/// <param name="allocMemProperty">memory allocation flag (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
/// VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT etc )</param>
/// <param name="image">The image the memory to be allocated for</param>
/// <param name="outMemRequirements">Return memory requirements if not NULL</param>
/// <returns>Return true if the allocation success</returns>
bool allocateImageDeviceMemory(VkDevice device, VkPhysicalDeviceMemoryProperties& deviceMemProperty,
                               VkMemoryPropertyFlagBits allocMemProperty, VkImage& outImage,
                               VkDeviceMemory& outMemory, VkMemoryRequirements* outMemRequirements);

/// <summary>Set image layout</summary>
/// <param name="cmd">Recording commandbuffer for image layout transition command.</param>
/// <param name="oldLayout"></param>
/// <param name="newLayout"></param>
/// <param name="image"></param>
/// <param name="aspectFlags"></param>
/// <param name="baseMipLevel"></param>
/// <param name="numMipLevels"></param>
/// <param name="baseArrayLayer"></param>
/// <param name="numArrayLayers"></param>
void setImageLayoutAndQueueOwnership(VkCommandBuffer cmdsrc, VkCommandBuffer cmddst,
                                     uint32 srcQueueFamily, uint32 dstQueueFamily,
                                     VkImageLayout oldLayout, VkImageLayout newLayout,
                                     VkImage image, uint32 baseMipLevel, uint32 numMipLevels,
                                     uint32 baseArrayLayer, uint32 numArrayLayers, VkImageAspectFlags aspect);

class CleanupObject_
{
	bool _destroyed;
	CleanupObject_(const CleanupObject_&) {} //deleted
public:
	void cleanup()
	{
		if (!_destroyed) { cleanup_(); }
	}
	CleanupObject_() : _destroyed(false) {}
	virtual ~CleanupObject_() {}
private:
	virtual void cleanup_() = 0;
};

typedef std::unique_ptr<CleanupObject_> CleanupObject;

class TextureUpdateCleanupObject_ : public CleanupObject_
{
	VkDevice device;
	std::vector<VkBuffer> destroyThese;
	std::vector<VkDeviceMemory> destroyTheseAsWell;
public:
	~TextureUpdateCleanupObject_()
	{
		cleanup();
	}
	TextureUpdateCleanupObject_(VkDevice device) : device(device) {}
	template<typename iterator>
	void addBuffers(iterator buffBegin, iterator buffEnd)
	{
		for (; buffBegin != buffEnd; ++buffBegin)
		{ destroyThese.push_back(*buffBegin); }
	}
	template<typename iterator>
	void addMemories(iterator memBegin, iterator memEnd)
	{
		for (; memBegin != memEnd; ++memBegin)
		{ destroyTheseAsWell.push_back(*memBegin); }
	}

	TextureUpdateCleanupObject_(TextureUpdateCleanupObject_&& rhs) :
		device(rhs.device)
	{
		swap(destroyThese, rhs.destroyThese);
		swap(destroyTheseAsWell, rhs.destroyTheseAsWell);
	}
	TextureUpdateCleanupObject_(TextureUpdateCleanupObject_& rhs) :
		device(rhs.device)
	{
		swap(destroyThese, rhs.destroyThese);
		swap(destroyTheseAsWell, rhs.destroyTheseAsWell);
	}
private:
	void cleanup_()
	{
		for (auto buffer : destroyThese)
		{
			if (buffer) { vk::DestroyBuffer(device, buffer, NULL); }
		}
		destroyThese.clear();
		for (auto memory : destroyTheseAsWell)
		{
			if (memory) { vk::FreeMemory(device, memory, NULL); }
		}
		destroyTheseAsWell.clear();
	}
};


/// <summary>Utility function to update an image's data. This function will record the update of the
/// image in the supplied command buffer but NOT submit the command buffer, hence allowing the user
/// to submit it at his own time.
/// IMPORTANT. Assumes image layout is VK_IMAGE_LAYOUT_DST_OPTIMAL
/// IMPORTANT. The cleanup object that is the return value of the function
/// must be kept alive as long until the moment that the relevant command buffer submission is finished.
/// Then it can be destroyed (or the cleanup function be called) to free any relevant resources.</summary>
/// <param name="context">The contenxt of the image</param>
/// <param name="updateParams">This object is a c-style array of areas and the data to upload.</param>
/// <param name="numUpdateParams">The number of ImageUpdateParam objects in <paramref name="updateParams"/>
/// </param>
/// <param name="numArraySlice">The number of array slices of the image</param>
/// <param name="srcFormat">The format of the image</param>
/// <param name="isCubeMap">Is the image a cubemap</param>
/// <param name="image">The image to update</param>
/// <returns>Return true if success</returns>
CleanupObject updateImageDeferred(VkDevice device, VkCommandBuffer cbuffTransfer,
                                  VkCommandBuffer cbuffTakeOwn,
                                  VkCommandBuffer cbuffRelinquishOwn,
                                  uint32 srcQueueFamily, uint32 dstQueueFamily,
                                  const VkPhysicalDeviceMemoryProperties& memprops,
                                  ImageUpdateParam* updateParams, uint32 numUpdateParams,
                                  VkFormat srcFormat, VkImageLayout layout,
                                  bool isCubeMap, VkImage image);

/// <summary>Utility function to update an image's data</summary>
/// <param name="context">The contenxt of the image</param>
/// <param name="updateParams">This object is a c-style array of areas and the data to upload.</param>
/// <param name="numUpdateParams">The number of ImageUpdateParam objects in <paramref name="updateParams"/>
/// </param>
/// <param name="numArraySlice">The number of array slices of the image</param>
/// <param name="srcFormat">The format of the image</param>
/// <param name="isCubeMap">Is the image a cubemap</param>
/// <param name="image">The image to update</param>
/// <returns>Return true if success</returns>
void updateImage(IPlatformContext& ctx, ImageUpdateParam* updateParams, uint32 numUpdateParams,
                 uint32 numArraySlice, VkFormat srcFormat, bool isCubeMap, VkImage image,
                 VkImageLayout currentLayout);

struct TextureUploadResultsData_
{
	/// <summary>The dimensions of the texture created</summary>
	types::ImageAreaSize textureSize;
	/// <summary>A native texture handle where the texture was uploaded</summary>
	native::HTexture_ image;
	/// <summary>The format of the created texture</summary>
	PixelFormat format;
	/// <summary>Will be set to 'true' if the file was of an uncompressed format unsupported by the
	/// platform, and it was (software) decompressed to a supported uncompressed format</summary>
	bool decompressed;
	Result result;
	TextureUploadResultsData_() : result(Result::UnknownError), decompressed(false) {}
};
//privatization of TextureUploadResults_
class TextureUploadResults : public CleanupObject_
{
	TextureUploadResultsData_ res;
public:
	TextureUploadResults(const TextureUploadResults& rhs) : res(rhs.res) {}
	TextureUploadResults(const TextureUploadResultsData_&res) : res(res) {}
	const native::HTexture_& getImage() const { return res.image; }
	types::ImageAreaSize getSize() const { return res.textureSize; }
	PixelFormat getPixelFormat() const { return res.format; }
	bool isDecompressed() const { return res.decompressed; }
	Result getResult() const { return res.result; }
private:
	void cleanup_() {}
};

//privatization of TextureUploadResults_

struct TextureUploadAsyncResultsData_ : public TextureUploadResultsData_
{
	VkFence fence;
	VkDevice device;
	VkCommandBuffer xferCmd;
	VkCommandPool xferPool;
	VkCommandBuffer ownCmd[2];
	VkCommandPool ownPool;
	CleanupObject updateCleanupData;
	TextureUploadAsyncResultsData_(): fence(VK_NULL_HANDLE), device(VK_NULL_HANDLE), xferCmd(VK_NULL_HANDLE),
		xferPool(VK_NULL_HANDLE), ownPool(VK_NULL_HANDLE)
	{
		ownCmd[0] = VK_NULL_HANDLE; ownCmd[1] = VK_NULL_HANDLE;
	}
	TextureUploadAsyncResultsData_(TextureUploadAsyncResultsData_&& rhs) :
		TextureUploadResultsData_(rhs),
		fence(rhs.fence), device(rhs.device), xferCmd(rhs.xferCmd),
		xferPool(rhs.xferPool), ownPool(rhs.ownPool),
		updateCleanupData(std::move(rhs.updateCleanupData))
	{
		ownCmd[0] = rhs.ownCmd[0];
		ownCmd[1] = rhs.ownCmd[1];
		rhs.ownPool = 0;
		rhs.ownCmd[0] = 0;
		rhs.ownCmd[1] = 0;
		rhs.fence = 0;
		rhs.device = 0;
		rhs.xferCmd = 0;
		rhs.xferPool = 0;
	}
private:
	TextureUploadAsyncResultsData_(const TextureUploadAsyncResultsData_& rhs);
	TextureUploadAsyncResultsData_& operator=(const TextureUploadAsyncResultsData_& rhs);

};

class TextureUploadAsyncResults_ : public CleanupObject_
{

	TextureUploadAsyncResultsData_ res;
	mutable bool done;
private:
	TextureUploadAsyncResults_(const TextureUploadAsyncResults_& rhs);
	TextureUploadAsyncResults_& operator=(const TextureUploadAsyncResults_& rhs);
public:
	TextureUploadAsyncResults_(TextureUploadAsyncResultsData_&& res) : res(std::move(res)), done(false) {}
	TextureUploadAsyncResults_(TextureUploadAsyncResults_&& rhs) : res(std::move(rhs.res)), done(rhs.done) {}
	const native::HTexture_& getImage()
	{
		if (!done && res.fence && res.result == Result::Success)
		{
			vk::WaitForFences(res.device, 1, &res.fence, true, std::numeric_limits<uint64_t>::max());
			done = true;
		}
		return res.image;
	}
	types::ImageAreaSize getSize() const { return res.textureSize; }
	PixelFormat getPixelFormat() const { return res.format; }
	bool isDecompressed() const { return res.decompressed; }
	Result getResult() const { return res.result; }
	VkFence& fence() { return res.fence; }
	void cleanup_()
	{
		if (res.fence != VK_NULL_HANDLE)
		{
			if (!done)
			{
				vk::WaitForFences(res.device, 1, &res.fence, true, std::numeric_limits<uint64>::max());
			}
			vk::DestroyFence(res.device, res.fence, NULL);
			res.fence = VK_NULL_HANDLE;
		}
		if (res.xferCmd)
		{
			vk::FreeCommandBuffers(res.device, res.xferPool, 1, &res.xferCmd);
			res.xferCmd = VK_NULL_HANDLE;
			res.xferPool = VK_NULL_HANDLE;
		}
		if (res.ownCmd[0])
		{
			assertion(res.ownCmd[1] != 0);
			vk::FreeCommandBuffers(res.device, res.ownPool, 2, res.ownCmd);
			res.ownCmd[0] = res.ownCmd[1] = VK_NULL_HANDLE;
			res.ownPool = VK_NULL_HANDLE;
		}
		res.updateCleanupData.reset();
	}


};
typedef std::auto_ptr<TextureUploadAsyncResults_> TextureUploadAsyncResults;

/// <summary>Upload a texture to the GPU and retrieve the into native handle. Image layout after upload is
/// VK_LAYOUT_SHADER_READ_ONLY_OPTIMAL</summary>
/// <param name="context">The PlatformContext to use to upload the texture.</param>
/// <param name="texture">The pvr::assets::texture to upload to the GPU</param>
/// <param name="allowDecompress">Set to true to allow to attempt to de-compress unsupported compressed textures.
/// The textures will be decompressed if ALL of the following are true: The texture is in a compressed format that
/// can be decompressed by the framework (PVRTC), the platform does NOT support this format (if it is hardware
/// supported, it will never be decompressed), and this flag is set to true. Default:true.</param>
/// <returns>A TextureUploadResults struct containing the result of the function as well as all required information
/// about the returned texture, including if the function succeeded, the handles to the texture and memory, the
/// size and format of the resulting texture, and if it was decompressed</returns>
TextureUploadResults textureUpload(IPlatformContext& ctx, const Texture& texture, bool allowDecompress);

/// <summary>Upload a texture to the GPU and retrieve the into native handle. Image layout after upload is
/// VK_LAYOUT_SHADER_READ_ONLY_OPTIMAL. Must wait on the fence before using.</summary>
/// <param name="context">The PlatformContext to use to upload the texture.</param>
/// <param name="texture">The pvr::assets::texture to upload to the GPU</param>
/// <param name="allowDecompress">Set to true to allow to attempt to de-compress unsupported compressed textures.
/// The textures will be decompressed if ALL of the following are true: The texture is in a compressed format that
/// can be decompressed by the framework (PVRTC), the platform does NOT support this format (if it is hardware
/// supported, it will never be decompressed), and this flag is set to true. Default:true.</param>
/// <returns>A TextureUploadResults struct containing the result of the function as well as all required information
/// about the returned texture, including if the function succeeded, the handles to the texture and memory, the
/// size and format of the resulting texture, and if it was decompressed</returns>
TextureUploadAsyncResults textureUploadDeferred(ISharedPlatformContext& ctx, const Texture& texture, bool allowDecompress);
>>>>>>> 1776432f... 4.3
}
}
}

//!\endcond