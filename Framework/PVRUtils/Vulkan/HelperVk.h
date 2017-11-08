#pragma once
#include "PVRCore/PVRCore.h"
#include "PVRCore/IO/FileStream.h"
#include "PVRAssets/Model.h"
#include "PVRAssets/PVRAssets.h"
#include "PVRAssets/TextureLoad.h"
#include "PVRAssets/FileIO/PFXParser.h"
#include "PVRVk/DeviceVk.h"
#include "PVRVk/InstanceVk.h"
#include "PVRVk/CommandBufferVk.h"
#include "PVRVk/ImageVk.h"
#include "PVRVk/SurfaceVk.h"
#include "ConvertToVkTypes.h"
#include "PVRVk/CommandPoolVk.h"
#include "PVRVk/QueueVk.h"
#include "PVRVk/RenderPassVk.h"
#include "PVRVk/FramebufferVk.h"
#include "PVRVk/SwapchainVk.h"
#include "PVRUtils/Vulkan/MemoryAllocator.h"

namespace pvr {
namespace utils {
//!\cond NO_DOXYGEN
VkImageAspectFlags inferAspectFromFormat(VkFormat format);
//!\endcond

/// <summary>Create a new buffer object and (optionally) allocate and bind memory for it</summary>
/// <param name="device">The device on which to create the buffer</param>
/// <param name="size">The total size of the buffer</param>
/// <param name="bufferUsage">All buffer usages for which this buffer will be valid</param>
/// <param name="memoryProps">The flags with which memory will be created. If 0 is passed, no memory will be allocated for this buffer.</param>
/// <param name="bufferCreateFlags">Buffer creation flags (see Vulkan spec)</param>
/// <param name="sharingExclusive">indicates whether the buffer is exclusive for some queues or can be used simultaneously multiple queues</param>
/// <param name="queueFamilyIndices">If not exclusive, indicates which queue families the buffer can be used by</param>
/// <param name="numQueueFamilyIndices">If not exclusive, the number of queue families for which this buffer is valid</param>
/// <returns>Return a valid object if success</returns>.
pvrvk::Buffer createBuffer(pvrvk::Device device, VkDeviceSize size, VkBufferUsageFlags bufferUsage,
                           VkMemoryPropertyFlags memoryProps, VkBufferCreateFlags bufferCreateFlags = VkBufferCreateFlags(0),
                           bool sharingExclusive = true, const uint32_t* queueFamilyIndices = nullptr,
                           uint32_t numQueueFamilyIndices = 0);

/// <summary>create 3d Image(sparse or with memory backing, depending on <paramref name="flags"/>.
/// User should not call bindMemory on the image if sparse flags are used.
/// <paramref name="allocMemFlags"/> is ignored if <paramref name="flags"/> contains
/// a sparse binding flag.)</summary>
/// <param name="device">The device on which to create the image</param>
/// <param name="imageType">The type of the created image (1D/2D/3D etc)</param>
/// <param name="format">The Image format</param>
/// <param name="dimension">Image dimension</param>
/// <param name="usage">Image usage flags</param>
/// <param name="flags">Image create flags</param>
/// <param name="layerSize">Image layer size</param>
/// <param name="samples">NUmber of samples</param>
/// <param name="allocMemFlags">Allocate device memory for this image. Only create
/// memory backing if it is a non sparse image and a valid memory flag.
/// Passing VkMemoryPropertyFlags(0) will not create memory object for this image</param>
/// <param name="sharingExclusive">Specifying the sharing mode of this image.
/// Setting it true means that only a single queue can access it therefore
/// the calle must exclusively transfer queue ownership of this image if they want separate queue family to access this image
/// and the <paramref name="queueFamilyIndices"/>, <paramref name="numQueueFamilyIndices"/> is ignored.
/// Setting it to false means multiple queue family can access this image at any point in time and requires
/// <paramref name="queueFamilyIndices"/>, <paramref name="numQueueFamilyIndices"/></param>
/// <param name="queueFamilyIndices">A c-style array containing the queue family indices that
/// this image is exclusive to</param>
/// <param name="numQueueFamilyIndices">The number of queues in <paramref name="queueFamilyIndices"/></param>
/// <returns> The created Imageobject on success, null Image on failure</returns>
pvrvk::Image createImage(
  pvrvk::Device device, VkImageType imageType, VkFormat format,
  const pvrvk::Extent3D& dimension, VkImageUsageFlags usage,
  VkImageCreateFlags flags = VkImageCreateFlags(0),
  const pvrvk::ImageLayersSize& layerSize = pvrvk::ImageLayersSize(),
  VkSampleCountFlags samples = VkSampleCountFlags::e_1_BIT,
  VkMemoryPropertyFlags allocMemFlags = VkMemoryPropertyFlags::e_DEVICE_LOCAL_BIT,
  bool sharingExclusive = true,
  const uint32_t* queueFamilyIndices = nullptr,
  uint32_t numQueueFamilyIndices = 0);

/// <summary>The CleanupObject_ class provides a generic mechanism whereby an object can be cleaned up via
/// providing an implementation for the virtual cleanup function</summary>
class CleanupObject_
{
private:
	bool _destroyed;
	virtual void cleanup_() = 0;

public:
	/// <summary>The cleanup function will carry out the object specific cleanup.</summary>
	void cleanup()
	{
		if (!_destroyed) { cleanup_(); _destroyed = true;}
	}

	/// <summary>Default constructor for the CleanupObject_ class.</summary>
	CleanupObject_() : _destroyed(false) {}

	/// <summary>Destructor. Virtual(for polymorphic use).</summary>
	virtual ~CleanupObject_() {}
};

/// <summary>The ImageCleanupObject_ class provides a specialised mechanism for cleaning up the buffers generating whilst uploading an image</summary>
class ImageCleanupObject_ : public CleanupObject_
{
private:
	std::vector<pvrvk::Buffer> destroyThese;

	void cleanup_()
	{
		destroyThese.clear();
	}
public:

	/// <summary>Destructor. Implementation calls the cleanup function which ensures any resources are cleaned up correctly.</summary>
	~ImageCleanupObject_()
	{
		cleanup();
	}

	/// <summary>Default constructor for the ImageCleanupObject_ class.</summary>
	ImageCleanupObject_() {}

	/// <summary>Adds a number of buffers to the the clean up object.</summary>
	/// <typeparam name="iterator">A buffer iterator.</typeparam>
	/// <param name="buffBegin">The first buffer to add.</param>
	/// <param name="buffEnd">The last buffer to add</param>
	template<typename iterator>
	void addBuffers(iterator buffBegin, iterator buffEnd)
	{
		for (; buffBegin != buffEnd; ++buffBegin)
		{
			destroyThese.push_back(*buffBegin);
		}
	}

	/// <summary>Move constructor for the ImageCleanupObject_ class.</summary>
	/// <param name="rhs">The ImageCleanupObject_ object to move from.</param>
	ImageCleanupObject_(ImageCleanupObject_&& rhs)
	{
		swap(destroyThese, rhs.destroyThese);
	}

	/// <summary>Copy constructor for the ImageCleanupObject_ class.</summary>
	/// <param name="rhs">The ImageCleanupObject_ object to copy from.</param>
	ImageCleanupObject_(ImageCleanupObject_& rhs)
	{
		swap(destroyThese, rhs.destroyThese);
	}
};

/// <summary>A shared pointer to a CleanupObject_ object which provides easier use of the class.</summary>
typedef std::shared_ptr<CleanupObject_> CleanupObject;

//!\cond NO_DOXYGEN
struct ImageUploadResults;
//!\endcond

/// <summary>The ImageUpdateResults structure provides an easy and convenient wrapper for the relevant results for any image update function.</summary>
struct ImageUpdateResults
{
public:
	/// <summary>Default constructor for the ImageUpdateResults structure.</summary>
	ImageUpdateResults() : result(pvr::Result::UnknownError) {}

	/// <summary>Constructor for the ImageUpdateResults structure.</summary>
	/// <param name="cleanupObjects">A cleanup object which will be used for cleaning up the resources used in the image update.</param>
	/// <param name="image">The resulting image.</param>
	/// <param name="result">The results of any image update function.</param>
	ImageUpdateResults(CleanupObject cleanupObjects, pvrvk::Image& image, pvr::Result result)
		:  updateCleanupData(cleanupObjects), image(image), result(result) {}

	/// <summary>Getter for the image returned via an image update function call.</summary>
	/// <returns>The image returned from an image update function call.</returns>
	pvrvk::Image getImage() const { return image; }

	/// <summary>Getter for the result of an image update function call.</summary>
	/// <returns>The pvr::Result returned from an image update function call.</returns>
	pvr::Result getResult() const { return result; }

	/// <summary>Copy constructor for the ImageUpdateResults structure.</summary>
	/// <param name="updateResults">Another ImageUpdateResults object to initialise this ImageUpdateResults from.</param>
	ImageUpdateResults(const ImageUpdateResults& updateResults)
	{
		image = updateResults.image;
		result = updateResults.result;
		updateCleanupData = updateResults.updateCleanupData;
	}
private:
	friend struct ::pvr::utils::ImageUploadResults;

	pvrvk::Image image;
	pvr::Result result;
	CleanupObject updateCleanupData;
};

/// <summary>The ImageUploadResults structure provides an easy and convenient wrapper for handling the results of an image upload.</summary>
struct ImageUploadResults
{
	/// <summary>Default constructor for the ImageUploadResults structure.</summary>
	ImageUploadResults() : result(pvr::Result::UnknownError), decompressed(false) {}

	/// <summary>Constructor for the ImageUploadResults structure.</summary>
	/// <param name="cleanupObject">A cleanup object which will be used for cleaning up the resources used in the image upload.</param>
	/// <param name="imageView">The resulting pvrvk::ImageView.</param>
	/// <param name="isDecompressed">Sets whether the resulting pvr::pvrvk::ImageView has been decompressed.</param>
	/// <param name="result">The results of any image upload function.</param>
	ImageUploadResults(CleanupObject& cleanupObject, pvrvk::ImageView imageView, bool isDecompressed, pvr::Result result) :
		updateCleanupData(cleanupObject), imageView(imageView), decompressed(isDecompressed), result(result) {}

	/// <summary>Constructor for the ImageUploadResults structure.</summary>
	/// <param name="updateResults">An ImageUpdateResults structure whose cleanup objects this ImageUploadResults will take responsibility for.</param>
	/// <param name="imageView">The resulting pvrvk::ImageView.</param>
	/// <param name="isDecompressed">Sets whether the resulting pvr::pvrvk::ImageView has been decompressed.</param>
	/// <param name="result">The results of any image upload function.</param>
	ImageUploadResults(ImageUpdateResults& updateResults, pvrvk::ImageView imageView, bool isDecompressed, pvr::Result result) :
		updateCleanupData(updateResults.updateCleanupData), imageView(imageView), decompressed(isDecompressed), result(result) {}

	/// <summary>Copy constructor for the ImageUploadResults structure.</summary>
	/// <param name="updateResults">Another ImageUploadResults object to initialise this ImageUploadResults from.</param>
	ImageUploadResults(const ImageUploadResults& updateResults)
	{
		imageView = updateResults.imageView;
		decompressed = updateResults.decompressed;
		this->result = updateResults.result;
		updateCleanupData = updateResults.updateCleanupData;
	}

	/// <summary>Getter for the result returned via an image upload function call.</summary>
	/// <returns>The result returned from an image upload function call.</returns>
	pvr::Result getResult()const { return result; }

	/// <summary>Getter for the image view returned via an image upload function call.</summary>
	/// <returns>The image view returned from an image upload function call.</returns>
	pvrvk::ImageView getImageView()const { return imageView; }

	/// <summary>Getter for whether the image returned via an image upload function call was decompressed.</summary>
	/// <returns>'True' if the image was decompressed during hte image upload function call.</returns>
	bool isDecompressed()const { return decompressed; }
private:
	/// <summary>A native texture handle where the texture was uploaded</summary>
	pvrvk::ImageView imageView;
	/// <summary>Will be set to 'true' if the file was of an uncompressed format unsupported by the
	/// platform, and it was (software) decompressed to a supported uncompressed format</summary>
	bool decompressed;
	pvr::Result result;
	CleanupObject updateCleanupData;
};

/// <summary>Create perspective matrix that trasform scenes that use Opengl Convention (+y up) to Vulkan Convention (+y down).</summary>
/// <param name="fovy">The field of view in the y dimension or the vertical angle.</param>
/// <param name="aspect">The aspect ratio.</param>
/// <param name="near1">The near z plane.</param>
/// <param name="far1">The far z plane.</param>
/// <param name="rotate">An amount to rotate the generated matrix by around the z axis.</param>
/// <returns>A created perspective matrix based on the fovy, aspect, near, far and rotated values.</returns>
inline glm::mat4 getPerspectiveMatrix(float fovy, float aspect, float near1, float far1, float rotate = .0f)
{
	glm::mat4 mat = glm::perspective(fovy, aspect, near1, far1);
	mat[1][1] *= -1.f;// negate the y axis's y component, because vulkan coordinate system is +y down
	return (rotate == 0.f ? mat : glm::rotate(rotate, glm::vec3(0.0f, 0.0f, 1.0f)) * mat);
}

/// <summary>Set image layout and queue family ownership</summary>
/// <param name="srccmd">The source command buffer from which to transition the image from.</param>
/// <param name="dstcmd">The destination command buffer from which to transition the image to.</param>
/// <param name="srcQueueFamily">srcQueueFamily is the source queue family for a queue family ownership transfer.</param>
/// <param name="dstQueueFamily">dstQueueFamily is the destination queue family for a queue family ownership transfer.</param>
/// <param name="oldLayout">An old image layout to transition from</param>
/// <param name="newLayout">A new image layout to transition to</param>
/// <param name="image">The image to transition</param>
/// <param name="baseMipLevel">The base mip level of the image to transition</param>
/// <param name="numMipLevels">The number of mip levels of the image to transition</param>
/// <param name="baseArrayLayer">The base array layer level of the image to transition</param>
/// <param name="numArrayLayers">The number of array layers of the image to transition</param>
/// <param name="aspect">The VkImageAspectFlags of the image to transition</param>
void setImageLayoutAndQueueFamilyOwnership(pvrvk::CommandBufferBase srccmd, pvrvk::CommandBufferBase dstcmd, uint32_t srcQueueFamily, uint32_t dstQueueFamily,
    VkImageLayout oldLayout, VkImageLayout newLayout, pvrvk::Image image, uint32_t baseMipLevel, uint32_t numMipLevels, uint32_t baseArrayLayer,
    uint32_t numArrayLayers, VkImageAspectFlags aspect);

/// <summary>Set image layout</summary>
/// <param name="image">The image to transition</param>
/// <param name="oldLayout">An old image layout to transition from</param>
/// <param name="newLayout">A new image layout to transition to</param>
/// <param name="transitionCmdBuffer">A command buffer to add the pipelineBarrier for the image transition.</param>
inline void setImageLayout(pvrvk::Image image, VkImageLayout oldLayout, VkImageLayout newLayout, pvrvk::CommandBufferBase transitionCmdBuffer)
{
	setImageLayoutAndQueueFamilyOwnership(transitionCmdBuffer, pvrvk::CommandBufferBase(), static_cast<uint32_t>(-1), static_cast<uint32_t>(-1),
	                                      oldLayout, newLayout, image, 0, image->getNumMipMapLevels(), 0,
	                                      static_cast<uint32_t>(image->getNumArrayLayers() * (1 + image->isCubeMap() * 5)), inferAspectFromFormat(image->getFormat()));
}

/// <summary>Uploads an image to GPU memory and returns the created image view and associated image.</summary>
/// <param name="device">The device to use to create the image and image view.</param>
/// <param name="texture">The source pvr::assets::Texture object from which to take the texture data.</param>
/// <param name="allowDecompress">Specifies whether the texture can be decompressed as part of the image upload.</param>
/// <param name="pool">A command pool from which to allocate a temporary command buffer to carry out the upload operations.</param>
/// <param name="queue">A queue to which the upload operations should be submitted to.</param>
/// <param name="usageFlags">A set of image usage flags for which the created image can be used for.</param>
/// <returns>The result of the image upload will be a created image view with its associated pvrvk::Image.</returns>
pvrvk::ImageView uploadImageAndSubmit(pvrvk::Device& device, const Texture& texture, bool allowDecompress, pvrvk::CommandPool& pool, pvrvk::Queue& queue,
                                      VkImageUsageFlags usageFlags = VkImageUsageFlags::e_SAMPLED_BIT, MemorySuballocator* allocator = NULL);

/// <summary>Uploads an image to gpu. The upload command and staging buffers are recorded in the commandbuffer, therefore
/// the caller must make sure the ImageUploadResult is valid until the command buffer submission.</summary>
/// <param name="device">The device to use to create the image and image view.</param>
/// <param name="texture">The source pvr::assets::Texture object from which to take the texture data.</param>
/// <param name="allowDecompress">Specifies whether the texture can be decompressed as part of the image upload.</param>
/// <param name="commandBuffer">A secondary command buffer to which the upload operations are be added. Note that the upload will not
/// be guranteed to be complete until the command buffer is submitted to a queue with appropriate synchronisation.</param>
/// <param name="usageFlags">A command buffer to add the pipelineBarrier for the image transition.</param>
/// <returns>An ImageUploadResults structure containing the results of the image upload.</returns>
ImageUploadResults uploadImage(pvrvk::Device& device, const Texture& texture, bool allowDecompress, pvrvk::SecondaryCommandBuffer& commandBuffer,
                               VkImageUsageFlags usageFlags = VkImageUsageFlags::e_SAMPLED_BIT, MemorySuballocator* allocator = NULL);

/// <summary>Upload image to gpu. The upload command and staging buffers are recorded in the commandbuffer, therefore
/// the calle must make sure the ImageUploadResult is valid until the command buffer submission.</summary>
/// <param name="device">The device to use to create the image and image view.</param>
/// <param name="texture">The source pvr::assets::Texture object from which to take the texture data.</param>
/// <param name="allowDecompress">Specifies whether the texture can be decompressed as part of the image upload.</param>
/// <param name="commandBuffer">A command buffer to which the upload operations should be added. Note that the upload will not
/// be guranteed to be complete until the command buffer is submitted to a queue with appropriate synchronisation.</param>
/// <param name="usageFlags">A command buffer to add the pipelineBarrier for the image transition.</param>
/// <returns>An ImageUploadResults structure containing the results of the image upload.</returns>
ImageUploadResults uploadImage(pvrvk::Device& device, const Texture& texture, bool allowDecompress, pvrvk::CommandBuffer& commandBuffer,
                               VkImageUsageFlags usageFlags = VkImageUsageFlags::e_SAMPLED_BIT, MemorySuballocator* allocator = NULL);

/// <summary>Load and upload image to gpu. The upload command and staging buffers are recorded in the commandbuffer, therefore
/// the calle must make sure the ImageUploadResult is valid until the command buffer submission.</summary>
/// <param name="device">The device to use to create the image and image view.</param>
/// <param name="fileName">The filename of a source texture from which to take the texture data.</param>
/// <param name="allowDecompress">Specifies whether the texture can be decompressed as part of the image upload.</param>
/// <param name="assetProvider">Specifies an asset provider to use for loading the texture from system memory.</param>
/// <param name="commandBuffer">A command buffer to which the upload operations should be added. Note that the upload will not
/// be guranteed to be complete until the command buffer is submitted to a queue with appropriate synchronisation.</param>
/// <param name="usageFlags">A command buffer to add the pipelineBarrier for the image transition.</param>
/// <param name="outAssetTexture">A pointer to a created pvr::assets::Texture.</param>
/// <returns>An ImageUploadResults structure containing the results of the image upload.</returns>
ImageUploadResults loadAndUploadImage(pvrvk::Device& device, const char* fileName, bool allowDecompress, pvrvk::CommandBuffer& commandBuffer,
                                      IAssetProvider& assetProvider, VkImageUsageFlags usageFlags = VkImageUsageFlags::e_SAMPLED_BIT,
                                      Texture* outAssetTexture = nullptr, MemorySuballocator* allocator = NULL);

/// <summary>Load and upload image to gpu. The upload command and staging buffers are recorded in the commandbuffer, therefore
/// the calle must make sure the ImageUploadResult is valid until the command buffer submission.</summary>
/// <param name="device">The device to use to create the image and image view.</param>
/// <param name="fileName">The filename of a source texture from which to take the texture data.</param>
/// <param name="allowDecompress">Specifies whether the texture can be decompressed as part of the image upload.</param>
/// <param name="commandBuffer">A secondary command buffer to which the upload operations should be added. Note that the upload will not
/// be guranteed to be complete until the command buffer is submitted to a queue with appropriate synchronisation.</param>
/// <param name="assetProvider">Specifies an asset provider to use for loading the texture from system memory.</param>
/// <param name="usageFlags">A command buffer to add the pipelineBarrier for the image transition.</param>
/// <param name="outAssetTexture">A pointer to a created pvr::assets::Texture.</param>
/// <returns>An ImageUploadResults structure containing the results of the image upload.</returns>
ImageUploadResults loadAndUploadImage(pvrvk::Device& device, const char* fileName, bool allowDecompress, pvrvk::SecondaryCommandBuffer& commandBuffer,
                                      IAssetProvider& assetProvider, VkImageUsageFlags usageFlags = VkImageUsageFlags::e_SAMPLED_BIT,
                                      Texture* outAssetTexture = nullptr, MemorySuballocator* allocator = NULL);

/// <summary>The ImageUpdateInfo struct.</summary>
struct ImageUpdateInfo
{
	// 1D/Array texture and common for rest
	int32_t       offsetX;  //!< Valid for all
	uint32_t      imageWidth;    //!< Valid for all
	uint32_t      dataWidth;    //!< Valid for all
	uint32_t      arrayIndex; //!< Valid for 1D,2D and Cube texture updates
	uint32_t      mipLevel; //!< Valid for all
	const void*   data;   //!< Valid for all
	uint32_t      dataSize; //!< Valid for all


	// 2D/ Array texture only
	int32_t       offsetY;  //!< Valid for 2D, 3D and Cube texture updates
	uint32_t      imageHeight;   //!< Valid for 2D, 3D and Cube texture updates
	uint32_t      dataHeight;   //!< Valid for 2D, 3D and Cube texture updates

	// cube/ Array Map only. Derive all states above
	uint32_t      cubeFace; //!< Valid for Cube texture updates only

	// 3D texture Only. Derive all states above Except arrayIndex
	int32_t       offsetZ;  //!< Valid for 3D texture updates only
	uint32_t      depth;    //!< Valid for texture updates only

	ImageUpdateInfo() : offsetX(0), imageWidth(1), dataWidth(1), mipLevel(0), data(0), dataSize(0), offsetY(0), imageHeight(1), dataHeight(1),
		cubeFace(0), offsetZ(0), depth(1) {}
};

/// <summary>Utility function to update an image's data. This function will record the update of the
/// image in the supplied command buffer but NOT submit the command buffer, hence allowing the user
/// to submit it at his own time.
/// IMPORTANT. Assumes image layout is VkImageLayout::e_DST_OPTIMAL
/// IMPORTANT. The cleanup object that is the return value of the function
/// must be kept alive as long until the moment that the relevant command buffer submission is finished.
/// Then it can be destroyed (or the cleanup function be called) to free any relevant resources.</summary>
/// <param name="device">The device used to create the image</param>
/// <param name="transferCommandBuffer">The command buffer into which the image update operations will be added.</param>
/// <param name="updateInfos">This object is a c-style array of areas and the data to upload.</param>
/// <param name="numUpdateInfos">The number of ImageUpdateInfo objects in </param>
/// <param name="format">The format of the image.</param>
/// <param name="layout">The final image layout for the image being updated.</param>
/// <param name="isCubeMap">Is the image a cubemap</param>
/// <param name="image">The image to update</param>
/// <returns>Returns an pvrvk::Image update results structure - ImageUpdateResults</returns>
ImageUpdateResults updateImage(pvrvk::Device& device, pvrvk::CommandBufferBase transferCommandBuffer, ImageUpdateInfo* updateInfos, uint32_t numUpdateInfos, VkFormat format,
                               VkImageLayout layout, bool isCubeMap, pvrvk::Image& image, MemorySuballocator* allocator = NULL);

/// <summary>Utility function to update a buffer's data.</summary>
/// <param name="device">The device used to create the buffer</param>
/// <param name="buffer">The buffer to map -> update -> unmap.</param>
/// <param name="data">The data to use in the update</param>
/// <param name="offset">The offset to use for the map -> update -> unmap</param>
/// <param name="size">The size of the data to be updated</param>
/// <param name="flushMemory">Boolean flag determining whether to flush the memory prior to the unmap</param>
/// <param name="invalidateMemory">Boolean flag determining whether to invalidate the memory prior to the unmap</param>
inline void updateBuffer(pvrvk::Device& device, pvrvk::Buffer& buffer, const void* data, VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE,
                         bool flushMemory = false, bool invalidateMemory = false)
{
	void* mapData = 0;
	if ((buffer->getDeviceMemory()->map(&mapData, offset, size) == VkResult::e_SUCCESS) && mapData)
	{
		memcpy(mapData, data, (size_t)size);
		if (flushMemory)
		{
			buffer->getDeviceMemory()->flushRange(offset, size);
		}

		if (invalidateMemory)
		{
			buffer->getDeviceMemory()->invalidateRange(offset, size);
		}

		buffer->getDeviceMemory()->unmap();
	}
}

/// <summary>Utility function for generating a texture atlas based on a set of images.</summary>
/// <param name="device">The device used to create the texture atlas.</param>
/// <param name="textures">A list of textures used to generate the texture atlas from.</param>
/// <param name="outUVs">A pointer to a set of UVs corresponding to the position of the images within the generated texture atlas.</param>
/// <param name="numTextures">The number of textures used for generating the texture atlas.</param>
/// <param name="outTexture">The generated texture atlas returned by the function</param>
/// <param name="outDescriptor">The texture header for the generated texture atlas</param>
/// <param name="cmdBuffer">A previously constructured command buffer which will be used by the utility function for various operations such as creating images.</param>
/// <returns>Return true if success</returns>
bool generateTextureAtlas(pvrvk::Device& device, const pvrvk::Image* textures, pvrvk::Rect2Df* outUVs, uint32_t numTextures,
                          pvrvk::ImageView* outTexture, TextureHeader* outDescriptor, pvrvk::CommandBufferBase cmdBuffer);

/// <summary>A structure encapsulating the set of queue flags required for a particular queue retrieved via the helper function 'createDeviceAndQueues'.
/// Optionally additionally providing a surface will indicate that the queue must support presentation via the provided surface.</summary>
struct QueuePopulateInfo
{
	/// <summary>The queue flags the queue must support.</summary>
	VkQueueFlags queueFlags;

	/// <summary>Indicates that the retrieved queue must support presentation to the provided surface.</summary>
	pvrvk::Surface surface;
};

/// <summary>A structure encapsulating the family id and queue id of a particular queue retrieved via the helper function 'createDeviceAndQueues'.
/// The family id corresponds to the family id the queue was retrieved from. The queue id corresponds to the particular queue index for the retrieved queue.</summary>
struct QueueAccessInfo
{
	/// <summary>The queue family identifier the queue with queueId was retrieved from.</summary>
	uint32_t familyId;

	/// <summary>The queue identifier for the retrieved queue in queue family with identifier familyId.</summary>
	uint32_t queueId;
};

/// <summary>Container for a list of device extensions to be used for initiailising a device using the helper function 'createDeviceAndQueues'.</summary>
struct DeviceExtensions
{
	std::vector<std::string> extensionStrings; //!< A list of device extensions
	/// <summary>Default constructor. Initialises a list of device extensions to be used by default when using the Framework.</summary>
	DeviceExtensions()
	{
		// enable the swap chain extension
		extensionStrings.push_back("VK_KHR_swapchain");

		// attempt to enable pvrtc extension
		extensionStrings.push_back("VK_IMG_format_pvrtc");

		// attempt to enable IMG cubic filtering
		extensionStrings.push_back("VK_IMG_filter_cubic");

		// if the build is Debug then enable the DEBUG_MARKER extension to aid with debugging
#ifdef DEBUG
		extensionStrings.push_back("VK_LUNARG_DEBUG_MARKER");
#endif

	}
};

/// <summary>Container for a list of instance layers to be used for initiailising an instance using the helper function 'createInstanceAndSurface'.</summary>
struct InstanceLayers
{
	/// <summary>A list of instance layers.</summary>
	std::vector<std::string> layersStrings;

	/// <summary>Default constructor. Initialises the list of instance layers based on whether the build is Debug/Release.</summary>
	/// <param name="forceLayers">A boolean flag which can be used to force the use of VK_LAYER_LUNARG_standard_validation even when in Releae builds.
	/// Note that the VK_LAYER_LUNARG_standard_validation layers will be enabled by default in Debug builds.</param>
	InstanceLayers(bool forceLayers =
#ifdef DEBUG
	                 true)
#else
	                 false)
#endif
	{
		// if the build is Debug then enable the LUNARG_standard_validation layer to aid with debugging
		if (forceLayers)
		{
			layersStrings.push_back("VK_LAYER_LUNARG_standard_validation");
		}
	}
};

/// <summary>Container for a list of instance extensions to be used for initiailising an instance using the helper function 'createInstanceAndSurface'.</summary>
struct InstanceExtensions
{
	std::vector<std::string> extensionStrings; //!< A list of instance extensions

	/// <summary>Default constructor. Initialises a list of instance extensions to be used by default when using the Framework.</summary>
	InstanceExtensions()
	{
		extensionStrings.push_back("VK_KHR_surface");
		extensionStrings.push_back("VK_KHR_display");
		extensionStrings.push_back("VK_KHR_win32_surface");
		extensionStrings.push_back("VK_KHR_android_surface");
		extensionStrings.push_back("VK_KHR_xlib_surface");
		extensionStrings.push_back("VK_KHR_xcb_surface");
		extensionStrings.push_back("VK_KHR_wayland_surface");
		extensionStrings.push_back("VK_KHR_get_physical_device_properties2");

		// if the build is Debug then enable the debug_report extension to aid with debugging
#ifdef DEBUG
		extensionStrings.push_back("VK_EXT_debug_report");
#endif
	}
};

/// <summary>Create the pvrvk::Device and the queues</summary>
/// <param name="physicalDevice">A physical device to use for creating the logical device.</param>
/// <param name="queueCreateFlags">A pointer to a list of QueuePopulateInfo structures specifying the required properties for each of the queues retrieved.</param>
/// <param name="numQueueCreateFlags">The number of QueuePopulateInfo structures provided.</param>
/// <param name="outAccessInfo">A pointer to a list of QueueAccessInfo structures specifying the properties for each of the queues retrieved.</param>
/// <param name="deviceExtensions">A DeviceExtensions structure which specifyies a list device extensions to try to enable.</param>
/// <returns>Return the created device</returns>
pvrvk::Device createDeviceAndQueues(pvrvk::PhysicalDevice physicalDevice, const QueuePopulateInfo* queueCreateFlags, uint32_t numQueueCreateFlags,
                                    QueueAccessInfo* outAccessInfo, const DeviceExtensions& deviceExtensions = DeviceExtensions());

/// <summary>Create a pvrvk::Swapchain using a pre-initialised pvrvk::Device and pvrvk::Surface choosing the color format of the swapchain images created from
/// the specified list of preferred color formats.</summary>
/// <param name="device">A logical device to use for creating the pvrvk::Swapchain.</param>
/// <param name="surface">A pvrvk::Surface from which surface capabilities, supported formats and presentation modes will be derived.</param>
/// <param name="displayAttributes">A set of display attributes from which certain properties will be taken such as width, height, vsync mode and
/// preferences for the number of pixels per channel.</param>
/// <param name="preferredColorFormats">A pointer to a list of preferred color formats from which the pvrvk::Swapchain color image format will be taken. Note that this
/// list must be exhaustive as if none are supported then no pvrvk::Swapchain will be created.</param>
/// <param name="numColorFormats">The number of color formats specified in the array pointed to by preferredColorFormats.</param>
/// <param name="swapchainImageUsageFlags">Specifies for what the swapchain images can be used for.</param>
/// <returns>Return the created pvrvk::Swapchain</returns>
pvrvk::Swapchain createSwapchain(pvrvk::Device& device, const pvrvk::Surface& surface, pvr::DisplayAttributes& displayAttributes, VkFormat* preferredColorFormats, uint32_t numColorFormats,
                                 VkImageUsageFlags swapchainImageUsageFlags = VkImageUsageFlags::e_COLOR_ATTACHMENT_BIT);

/// <summary>Create a pvrvk::Swapchain using a pre-initialised pvrvk::Device and pvrvk::Surface.</summary>
/// <param name="device">A logical device to use for creating the pvrvk::Swapchain.</param>
/// <param name="surface">A pvrvk::Surface from which surface capabilities, supported formats and presentation modes will be derived.</param>
/// <param name="displayAttributes">A set of display attributes from which certain properties will be taken such as width, height, vsync mode and
/// preferences for the number of pixels per channel.</param>
/// <param name="swapchainImageUsageFlags">Specifies for what the swapchain images can be used for.</param>
/// <returns>Return the created pvrvk::Swapchain</returns>
pvrvk::Swapchain createSwapchain(pvrvk::Device& device, const pvrvk::Surface& surface, pvr::DisplayAttributes& displayAttributes,
                                 VkImageUsageFlags swapchainImageUsageFlags = VkImageUsageFlags::e_COLOR_ATTACHMENT_BIT);

/// <summary>Create a pvrvk::Swapchain and corresponding number of depth stencil images using a pre-initialised pvrvk::Device and pvrvk::Surface
/// choosing the color and depth stencil format of the images created from the specified list of preferred color and depth stencil formats.</summary>
/// <param name="device">A logical device to use for creating the pvrvk::Swapchain and depth stencil images.</param>
/// <param name="surface">A pvrvk::Surface from which surface capabilities, supported formats and presentation modes will be derived.</param>
/// <param name="displayAttributes">A set of display attributes from which certain properties will be taken such as width, height, vsync mode and
/// preferences for the number of pixels per channel.</param>
/// <param name="outSwapchain">The created swapchain will be returned by reference.</param>
/// <param name="outDepthStencil">A Multi<pvrvk::ImageView> containing the created depth stencil images. This Multi will have size equal to the number of swapchain images.</param>
/// <param name="preferredColorFormats">A pointer to a list of preferred color formats from which the pvrvk::Swapchain color image format will be taken. Note that this
/// list must be exhaustive as if none are supported then no pvrvk::Swapchain will be created.</param>
/// <param name="numColorFormats">The number of color formats specified in the array pointed to by preferredColorFormats.</param>
/// <param name="preferredDepthStencilFormats">A pointer to a list of preferred depth stencil formats from which the depth stencil image format will be taken. Note that this
/// list must be exhaustive as if none are supported then no depth stencil images will be created.</param>
/// <param name="numDepthStencilFormats">The number of depth stencil formats specified in the array pointed to by preferredDepthStencilFormats.</param>
/// <param name="swapchainImageUsageFlags">Specifies for what the swapchain images can be used for.</param>
/// <param name="dsImageUsageFlags">Specifies for what the depth stencil images can be used for.</param>
/// <returns>Return 'True' if the pvrvk::Swapchain and depth stencil images were created successfully.</returns>
bool createSwapchainAndDepthStencilImageView(pvrvk::Device& device, const pvrvk::Surface& surface, pvr::DisplayAttributes& displayAttributes,
    pvrvk::Swapchain& outSwapchain, Multi<pvrvk::ImageView>& outDepthStencil, VkFormat* preferredColorFormats, uint32_t numColorFormats,
    VkFormat* preferredDepthStencilFormats, uint32_t numDepthStencilFormats, const VkImageUsageFlags& swapchainImageUsageFlags = VkImageUsageFlags::e_COLOR_ATTACHMENT_BIT,
    const VkImageUsageFlags& dsImageUsageFlags = VkImageUsageFlags::e_DEPTH_STENCIL_ATTACHMENT_BIT | VkImageUsageFlags::e_TRANSIENT_ATTACHMENT_BIT);

/// <summary>Create a pvrvk::Swapchain and corresponding number of depth stencil images using a pre-initialised pvrvk::Device and pvrvk::Surface.</summary>
/// <param name="device">A logical device to use for creating the pvrvk::Swapchain.</param>
/// <param name="surface">A pvrvk::Surface from which surface capabilities, supported formats and presentation modes will be derived.</param>
/// <param name="displayAttributes">A set of display attributes from which certain properties will be taken such as width, height, vsync mode and
/// preferences for the number of pixels per channel.</param>
/// <param name="outSwapchain">The created swapchain will be returned by reference.</param>
/// <param name="outDepthStencil">A Multi<pvrvk::ImageView> containing the created depth stencil images. This Multi will have size equal to the number of swapchain images.</param>
/// <param name="swapchainImageUsageFlags">Specifies for what the swapchain images can be used for.</param>
/// <param name="dsImageUsageFlags">Specifies for what the depth stencil images can be used for.</param>
/// <returns>Return 'True' if the pvrvk::Swapchain and depth stencil images were created successfully.</returns>
bool createSwapchainAndDepthStencilImageView(pvrvk::Device& device, const pvrvk::Surface& surface, pvr::DisplayAttributes& displayAttributes,
    pvrvk::Swapchain& outSwapchain, Multi<pvrvk::ImageView>& outDepthStencil, const VkImageUsageFlags& swapchainImageUsageFlags = VkImageUsageFlags::e_COLOR_ATTACHMENT_BIT,
    const VkImageUsageFlags& dsImageUsageFlags = VkImageUsageFlags::e_DEPTH_STENCIL_ATTACHMENT_BIT | VkImageUsageFlags::e_TRANSIENT_ATTACHMENT_BIT);

/// <summary> Create a pvrvk::Framebuffer and Renderpass to use for 'default' rendering to the 'onscreen' color images with following config
/// Renderpass:
///     Attachment0: ColorAttachment
///         swapchain image,
///         finalLayout - PresentSrcKHR
///         LoadOp - Clear
///         StoreOp - Store
///     Attachment1: DepthStencilAttachment
///         finalLayout - DepthStencilAttachmentOptimal
///         LoadOp - Clear
///         StoreOp - Store
/// </summary>
/// <param name="swapchain">A pre-created swapchain object from which the device will be taken for creating the framebuffer and renderpass. The swapchain
/// image formats and dimensions will also be taken from the swapchain.</param>
/// <param name="depthStencilImages">A pointer to an array of pvrvk::ImageView objects corresponding to an image to use as the depth stencil image per swap chain.</param>
/// <param name="outFramebuffers">The created framebuffers will be returned by reference as part of outFramebuffers with each framebuffer corresponding to a single swap chain.</param>
/// <param name="outRenderPass">The created renderpass will be returned by reference.</param>
/// <param name="initialSwapchainLayout">Initial Layouts of the swapchain image</param>
/// <param name="initialDepthStencilLayout">Initial Layouts of the depthstencil image</param>
/// <returns>Return 'True' if the framebuffer and renderpass were created successfully.</returns>
inline bool createOnscreenFramebufferAndRenderpass(pvrvk::Swapchain& swapchain, pvrvk::ImageView* depthStencilImages, Multi<pvrvk::Framebuffer>& outFramebuffers, pvrvk::RenderPass& outRenderPass,
    VkImageLayout initialSwapchainLayout = VkImageLayout::e_UNDEFINED, VkImageLayout initialDepthStencilLayout = VkImageLayout::e_UNDEFINED)
{
	pvrvk::FramebufferCreateInfo framebufferInfos[pvrvk::FrameworkCaps::MaxSwapChains];
	pvrvk::RenderPassCreateInfo rpInfo;
	rpInfo.setAttachmentDescription(0, pvrvk::AttachmentDescription::createColorDescription(swapchain->getImageFormat(),
	                                initialSwapchainLayout, VkImageLayout::e_PRESENT_SRC_KHR));

	pvrvk::SubPassDescription subpass;
	subpass.setColorAttachmentReference(0, pvrvk::AttachmentReference(0, VkImageLayout::e_COLOR_ATTACHMENT_OPTIMAL));
	if (depthStencilImages != nullptr)
	{
		rpInfo.setAttachmentDescription(1, pvrvk::AttachmentDescription::createDepthStencilDescription(
		                                  depthStencilImages[0]->getImage()->getFormat(), initialDepthStencilLayout));
		subpass.setDepthStencilAttachmentReference(pvrvk::AttachmentReference(1, VkImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL));
	}

	pvrvk::SubPassDependency dependencies[2] =
	{
		pvrvk::SubPassDependency(pvrvk::SubpassExternal, 0, VkPipelineStageFlags::e_BOTTOM_OF_PIPE_BIT,
		                         VkPipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT, VkAccessFlags::e_MEMORY_READ_BIT,
		                         VkAccessFlags::e_COLOR_ATTACHMENT_READ_BIT | VkAccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT,
		                         VkDependencyFlags::e_BY_REGION_BIT),

		pvrvk::SubPassDependency(0, pvrvk::SubpassExternal,
		                         VkPipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT, VkPipelineStageFlags::e_BOTTOM_OF_PIPE_BIT,
		                         VkAccessFlags::e_COLOR_ATTACHMENT_READ_BIT | VkAccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT,
		                         VkAccessFlags::e_MEMORY_READ_BIT, VkDependencyFlags::e_BY_REGION_BIT)
	};
	rpInfo.addSubPassDependencies(dependencies, ARRAY_SIZE(dependencies));

	rpInfo.setSubPass(0, subpass);
	outRenderPass = swapchain->getDevice()->createRenderPass(rpInfo);
	if (!outRenderPass.isValid())
	{
		return false;
	}
	for (uint32_t i = 0; i < swapchain->getSwapchainLength(); ++i)
	{
		framebufferInfos[i].setAttachment(0, swapchain->getImageView(i));
		framebufferInfos[i].setDimensions(swapchain->getDimension());
		if (depthStencilImages)
		{
			framebufferInfos[i].setAttachment(1, depthStencilImages[i]);
		}
		framebufferInfos[i].setRenderPass(outRenderPass);
		outFramebuffers[i] = swapchain->getDevice()->createFramebuffer(framebufferInfos[i]);
	}
	return true;
}

/// <summary> Create a pvrvk::Framebuffer and Renderpass to use for 'default' rendering to the 'onscreen' color images with following config
/// Renderpass:
///     ColorAttachment0:
///         swapchain image,
///         finalLayout - PresentSrcKHR
///         LoadOp - Clear
///         StoreOp - Store
///     DepthStencilAttachment:
///         finalLayout - DepthStencilAttachmentOptimal
///         LoadOp - Clear
///         StoreOp - Store
/// Subpass0
/// </summary>
/// <param name="swapchain">A pre-created swapchain object from which the device will be taken for creating the framebuffer and renderpass. The swapchain
/// image formats and dimensions will also be taken from the swapchain.</param>
/// <param name="depthStencilImages">A pointer to an array of pvrvk::ImageView objects corresponding to an image to use as the depth stencil image per swap chain.</param>
/// <param name="outFramebuffers">The created framebuffers will be returned by reference as part of outFramebuffers with each framebuffer corresponding to a single swap chain.</param>
/// <param name="initialSwapchainLayout">Initial Layouts of the swapchain image</param>
/// <param name="initialDepthStencilLayout">Initial Layouts of the depthstencil image</param>
/// <returns>Return 'True' if the framebuffer and renderpass were created successfully.</returns>
/// <param name="initialSwapchainLayout">Initial Layouts of the swapchain image</param>
/// <param name="initialDepthStencilLayout">Initial Layouts of the depthstencil image</param>
/// <remarks>The renderpass will not be returned directly but can instead be retrieved via a call to outFramebuffers[i].getRenderpass()</remarks>
inline bool createOnscreenFramebufferAndRenderpass(pvrvk::Swapchain& swapchain, pvrvk::ImageView* depthStencilImages, Multi<pvrvk::Framebuffer>& outFramebuffers,
    VkImageLayout initialSwapchainLayout = VkImageLayout::e_UNDEFINED, VkImageLayout initialDepthStencilLayout = VkImageLayout::e_UNDEFINED)
{
	pvrvk::RenderPass dummy;
	return createOnscreenFramebufferAndRenderpass(swapchain, depthStencilImages, outFramebuffers, dummy, initialSwapchainLayout,
	       initialDepthStencilLayout);
}

/// <summary>Fills out a pvrvk::ViewportStateCreateInfo structure setting parameters for a 'default' viewport and scissor based on the specified frame buffer dimensions.</summary>
/// <param name="framebuffer">An input Framebuffer object from which to take dimensions used to initialise a pvrvk::ViewportStateCreateInfo structure.</param>
/// <param name="outCreateInfo">A pvrvk::ViewportStateCreateInfo structure which will have its viewport and scissor members set based on the framebuffers dimensions.</param>
inline void populateViewportStateCreateInfo(const pvrvk::Framebuffer& framebuffer, pvrvk::ViewportStateCreateInfo& outCreateInfo)
{
	outCreateInfo.setViewportAndScissor(0, pvrvk::Viewport(0.f, 0.f, static_cast<float>(framebuffer->getDimensions().width),
	                                    static_cast<float>(framebuffer->getDimensions().height)), pvrvk::Rect2Di(0, 0, framebuffer->getDimensions().width, framebuffer->getDimensions().height));
}

/// <summary>Represents a shader Explicit binding, tying a Semantic name to an Attribute Index.</summary>
struct VertexBindings
{
	/// <summary>Effect semantic.</summary>
	std::string semanticName;

	/// <summary>Binding id.</summary>
	int16_t binding;
};

/// <summary>Represents a shader Reflective binding, tying a Semantic name to an Attribute variable name.</summary>
struct VertexBindings_Name
{
	/// <summary>Effect semantic.</summary>
	StringHash semantic;

	/// <summary>Shader attribute name.</summary>
	StringHash variableName;
};

/// <summary>Fills out input assembly and vertex input state structures using a mesh and a list of corresponding VertexBindings.</summary>
/// <param name="mesh">A mesh from which to retrieve vertex attributes, vertex buffer strides and primitive topology information from.</param>
/// <param name="bindingMap">A pointer to an array of VertexBindings structures which specify the semantic names and binding indices of any vertex attributes to retrieve.</param>
/// <param name="numBindings">Specifies the number of VertexBindings structures in the array pointed to by bindingMap.</param>
/// <param name="vertexCreateInfo">A pvrvk::PipelineVertexInputStateCreateInfo structure which will be filled by this utility function.</param>
/// <param name="inputAssemblerCreateInfo">A pvrvk::InputAssemblerStateCreateInfo structure which will be filled by this utility function.</param>
/// <param name="numOutBuffers">A pointer to an unsigned integer which will set to specify the number of buffers required to create
/// buffers for to use the mesh vertex attributes.</param>
inline void populateInputAssemblyFromMesh(const assets::Mesh& mesh, const VertexBindings* bindingMap, uint16_t numBindings,
    pvrvk::PipelineVertexInputStateCreateInfo& vertexCreateInfo, pvrvk::InputAssemblerStateCreateInfo& inputAssemblerCreateInfo, uint16_t* numOutBuffers = nullptr)
{
	vertexCreateInfo.clear();
	if (numOutBuffers) { *numOutBuffers = 0; }
	int16_t current = 0;
	while (current < numBindings)
	{
		auto attr = mesh.getVertexAttributeByName(bindingMap[current].semanticName.c_str());
		if (attr)
		{
			VertexAttributeLayout layout = attr->getVertexLayout();
			uint32_t stride = mesh.getStride(attr->getDataIndex());
			if (numOutBuffers) { *numOutBuffers = static_cast<uint16_t>(std::max<int32_t>(attr->getDataIndex() + 1, *numOutBuffers)); }

			const pvrvk::VertexInputAttributeDescription attribDesc(bindingMap[current].binding, attr->getDataIndex(),
			    convertToVkVertexInputFormat(layout.dataType, layout.width), layout.offset);

			const pvrvk::VertexInputBindingDescription bindingDesc(attr->getDataIndex(), stride, VkVertexInputRate::e_VERTEX);
			vertexCreateInfo.addInputAttribute(attribDesc).addInputBinding(bindingDesc);
		}
		else
		{
			Log("Could not find Attribute with Semantic %s in the supplied mesh. Will render without binding it, erroneously.",
			    bindingMap[current].semanticName.c_str());
		}
		++current;
	}
	inputAssemblerCreateInfo.setPrimitiveTopology(convertToVk(mesh.getMeshInfo().primitiveType));
}

/// <summary>Fills out input assembly and vertex input state structures using a mesh and a list of corresponding VertexBindings_Name.</summary>
/// <param name="mesh">A mesh from which to retrieve vertex attributes, vertex buffer strides and primitive topology information from.</param>
/// <param name="bindingMap">A pointer to an array of VertexBindings_Name structures which specify the semantic and binding names of any vertex attributes to retrieve.</param>
/// <param name="numBindings">Specifies the number of VertexBindings structures in the array pointed to by bindingMap.</param>
/// <param name="vertexCreateInfo">A pvrvk::PipelineVertexInputStateCreateInfo structure which will be filled by this utility function.</param>
/// <param name="inputAssemblerCreateInfo">A pvrvk::InputAssemblerStateCreateInfo structure which will be filled by this utility function.</param>
/// <param name="numOutBuffers">A pointer to an unsigned integer which will set to specify the number of buffers required to create
/// buffers for to use the mesh vertex attributes.</param>
inline void populateInputAssemblyFromMesh(const assets::Mesh& mesh, const VertexBindings_Name* bindingMap,
    uint16_t numBindings, pvrvk::PipelineVertexInputStateCreateInfo& vertexCreateInfo,
    pvrvk::InputAssemblerStateCreateInfo& inputAssemblerCreateInfo, uint16_t* numOutBuffers = nullptr)
{
	vertexCreateInfo.clear();
	if (numOutBuffers) { *numOutBuffers = 0; }
	int16_t current = 0;
	//In this scenario, we will be using our own indexes instead of user provided ones, correlating them by names.
	vertexCreateInfo.clear();
	while (current < numBindings)
	{
		auto attr = mesh.getVertexAttributeByName(bindingMap[current].semantic);
		if (attr)
		{
			VertexAttributeLayout layout = attr->getVertexLayout();
			uint32_t stride = mesh.getStride(attr->getDataIndex());

			if (numOutBuffers) { *numOutBuffers = static_cast<uint16_t>(std::max<int32_t>(attr->getDataIndex() + 1, *numOutBuffers)); }
			const pvrvk::VertexInputAttributeDescription attribDesc(current, attr->getDataIndex(),
			    convertToVkVertexInputFormat(layout.dataType, layout.width), layout.offset);

			const pvrvk::VertexInputBindingDescription bindingDesc(attr->getDataIndex(), stride, VkVertexInputRate::e_VERTEX);
			vertexCreateInfo.addInputAttribute(attribDesc).addInputBinding(bindingDesc);
			inputAssemblerCreateInfo.setPrimitiveTopology(convertToVk(mesh.getMeshInfo().primitiveType));
		}
		else
		{
			Log("Could not find Attribute with Semantic %s in the supplied mesh. Will render without binding it, erroneously.",
			    bindingMap[current].semantic.c_str());
		}
		++current;
	}
}

/// <summary>Auto generates a single VBO and a single IBO from all the vertex data of a mesh.</summary>
/// <param name="device">The device where the buffers will be generated on</param>
/// <param name="mesh">The mesh whose data will populate the buffers</param>
/// <param name="outVbo">The VBO handle where the data will be put. No buffer needs to have been created on the
/// handle</param>
/// <param name="outIbo">The IBO handle where the data will be put. No buffer needs to have been created on the
/// handle. If no face data is present on the mesh, the handle will be null.</param>
/// <remarks>This utility function will read all vertex data from a mesh's data elements and create a single VBO.
/// It is commonly used for a single set of interleaved data. If data are not interleaved, they will be packed on
/// the same VBO, each interleaved block (Data element on the mesh) will be appended at the end of the buffer, and
/// the offsets will need to be calculated by the user when binding the buffer.</remarks>
inline void createSingleBuffersFromMesh(pvrvk::Device& device, const assets::Mesh& mesh,
                                        pvrvk::Buffer& outVbo,  pvrvk::Buffer& outIbo)
{
	size_t total = 0;
	for (uint32_t i = 0; i < mesh.getNumDataElements(); ++i)
	{
		total += mesh.getDataSize(i);
	}

	outVbo = createBuffer(device, static_cast<uint32_t>(mesh.getDataSize(0)),
	                      VkBufferUsageFlags::e_VERTEX_BUFFER_BIT, VkMemoryPropertyFlags::e_HOST_VISIBLE_BIT);

	size_t current = 0;
	for (uint32_t i = 0; i < mesh.getNumDataElements(); ++i)
	{
		updateBuffer(device, outVbo, static_cast<const void*>(mesh.getData(i)),
		             static_cast<uint32_t>(current), static_cast<uint32_t>(mesh.getDataSize(i)), true);
		current += mesh.getDataSize(i);
	}

	if (mesh.getNumFaces())
	{
		outIbo = createBuffer(device, static_cast<uint32_t>(mesh.getFaces().getDataSize()),
		                      VkBufferUsageFlags::e_INDEX_BUFFER_BIT, VkMemoryPropertyFlags::e_HOST_VISIBLE_BIT);
		updateBuffer(device, outIbo, static_cast<const void*>(mesh.getFaces().getData()),
		             0, mesh.getFaces().getDataSize(), true);
	}

	else
	{
		outIbo.reset();
	}
}

/// <summary>Auto generates a set of VBOs and a single IBO from all the vertex data of a mesh.</summary>
/// <param name="device">The device where the buffers will be generated on</param>
/// <param name="mesh">The mesh whose data will populate the buffers</param>
/// <param name="outVbos">Reference to a std::vector of VBO handles where the data will be put. Buffers will be appended
/// at the end.</param>
/// <param name="outIbo">The IBO handle where the data will be put. No buffer needs to have been created on the
/// handle. If no face data is present on the mesh, the handle will be null.</param>
/// <remarks>This utility function will read all vertex data from the mesh and create one pvrvk::Buffer for each data
/// element (block of interleaved data) in the mesh. It is thus commonly used for for meshes containing multiple
/// sets of interleaved data (for example, a VBO with static and a VBO with streaming data).</remarks>
inline void createMultipleBuffersFromMesh(pvrvk::Device& device, const assets::Mesh& mesh,
    std::vector<pvrvk::Buffer>& outVbos, pvrvk::Buffer& outIbo)
{
	for (uint32_t i = 0; i < mesh.getNumDataElements(); ++i)
	{
		outVbos.push_back(createBuffer(device, static_cast<uint32_t>(mesh.getDataSize(i)),
		                               VkBufferUsageFlags::e_VERTEX_BUFFER_BIT, VkMemoryPropertyFlags::e_HOST_VISIBLE_BIT));
		updateBuffer(device, outVbos.back(), static_cast<const void*>(mesh.getData(i)),
		             0, static_cast<uint32_t>(mesh.getDataSize(0)), true);
	}
	if (mesh.getNumFaces())
	{
		outIbo = createBuffer(device, mesh.getFaces().getDataSize(),
		                      VkBufferUsageFlags::e_INDEX_BUFFER_BIT, VkMemoryPropertyFlags::e_HOST_VISIBLE_BIT);
		updateBuffer(device, outIbo, static_cast<const void*>(mesh.getFaces().getData()),
		             0, mesh.getFaces().getDataSize(), true);
	}
}

/// <summary>Auto generates a set of VBOs and a set of IBOs from the vertex data of multiple meshes and uses
/// std::inserter provided by the user to insert them to any container.</summary>
/// <param name="device">The device where the buffers will be generated on</param>
/// <param name="meshIter">Iterator for a collection of meshes.</param>
/// <param name="meshIterEnd">End Iterator for meshIter.</param>
/// <param name="outVbos">std::inserter for a collection of pvrvk::Buffer handles. It will be used to insert one VBO per mesh.
/// </param>
/// <param name="outIbos">std::inserter for a collection of pvrvk::Buffer handles. It will be used to insert one IBO per mesh.
/// If face data is not present on the mesh, a null handle will be inserted.</param>
/// <remarks>This utility function will read all vertex data from a mesh's data elements and create a single VBO.
/// It is commonly used for a single set of interleaved data (mesh.getNumDataElements() == 1). If more data
/// elements are present (i.e. more than a single interleaved data element) , they will be packed in the sameVBO,
/// with each interleaved block (Data element ) appended at the end of the buffer. It is then the user's
/// responsibility to use the buffer correctly with the API (for example use bindbufferbase and similar) with the
/// correct offsets. The std::inserter this function requires can be created from any container with an insert()
/// function with (for example, for insertion at the end of a vector) std::inserter(std::vector,
/// std::vector::end()) .</remarks>
template<typename MeshIterator_, typename VboInsertIterator_, typename IboInsertIterator_>
inline void createSingleBuffersFromMeshes(pvrvk::Device& device,
    MeshIterator_ meshIter, MeshIterator_ meshIterEnd, VboInsertIterator_ outVbos,
    IboInsertIterator_ outIbos)
{
	int i = 0;
	while (meshIter != meshIterEnd)
	{
		size_t total = 0;
		for (uint32_t ii = 0; ii < meshIter->getNumDataElements(); ++ii)
		{
			total += meshIter->getDataSize(ii);
		}

		pvrvk::Buffer vbo = createBuffer(device ,static_cast<uint32_t>(total),
		                    VkBufferUsageFlags::e_VERTEX_BUFFER_BIT, VkMemoryPropertyFlags::e_HOST_VISIBLE_BIT);
		size_t current = 0;
		for (size_t ii = 0; ii < meshIter->getNumDataElements(); ++ii)
		{
			pvr::utils::updateBuffer(device, vbo, (const void*)meshIter->getData(static_cast<uint32_t>(ii)), static_cast<uint32_t>(current),
			                         static_cast<uint32_t>(meshIter->getDataSize(static_cast<uint32_t>(ii))), true);
			current += meshIter->getDataSize(static_cast<uint32_t>(ii));
		}

		outVbos = vbo;
		if (meshIter->getNumFaces())
		{
			pvrvk::Buffer ibo = createBuffer(device, meshIter->getFaces().getDataSize(),
			                    VkBufferUsageFlags::e_INDEX_BUFFER_BIT, VkMemoryPropertyFlags::e_HOST_VISIBLE_BIT);
			pvr::utils::updateBuffer(device, ibo, static_cast<const void*>(meshIter->getFaces().getData()), 0, meshIter->getFaces().getDataSize(), true);
			outIbos = ibo;
		}
		else
		{
			outIbos = pvrvk::Buffer();
		}
		++outVbos;
		++outIbos;
		++i;
		++meshIter;
	}
}

/// <summary>Auto generates a set of VBOs and a set of IBOs from the vertex data of multiple meshes and insert them
/// at the specified spot in a user-provided container.</summary>
/// <param name="device">The device where the buffers will be generated on</param>
/// <param name="meshIter">Iterator for a collection of meshes.</param>
/// <param name="meshIterEnd">End Iterator for meshIter.</param>
/// <param name="outVbos">Collection of pvrvk::Buffer handles. It will be used to insert one VBO per mesh.</param>
/// <param name="outIbos">Collection of pvrvk::Buffer handles. It will be used to insert one IBO per mesh. If face data is
/// not present on the mesh, a null handle will be inserted.</param>
/// <param name="vbos_where">Iterator on outVbos - the position where the insertion will happen.</param>
/// <param name="ibos_where">Iterator on outIbos - the position where the insertion will happen.</param>
/// <remarks>This utility function will read all vertex data from a mesh's data elements and create a single VBO.
/// It is commonly used for a single set of interleaved data (mesh.getNumDataElements() == 1). If more data
/// elements are present (i.e. more than a single interleaved data element) , they will be packed in the sameVBO,
/// with each interleaved block (Data element ) appended at the end of the buffer. It is then the user's
/// responsibility to use the buffer correctly with the API (for example use bindbufferbase and similar) with the
/// correct offsets.</remarks>
template<typename MeshIterator_, typename VboContainer_, typename IboContainer_>
inline void createSingleBuffersFromMeshes(pvrvk::Device& device, MeshIterator_ meshIter,
    MeshIterator_ meshIterEnd, VboContainer_& outVbos, typename VboContainer_::iterator vbos_where,
    IboContainer_& outIbos, typename IboContainer_::iterator ibos_where)
{
	createSingleBuffersFromMeshes(device, meshIter, meshIterEnd,
	                              std::inserter(outVbos, vbos_where), std::inserter(outIbos, ibos_where));
}

/// <summary>Auto generates a set of VBOs and a set of IBOs from the vertex data of the meshes of a model and
/// inserts them into containers provided by the user using std::inserters.</summary>
/// <param name="device">The device device where the buffers will be generated on</param>
/// <param name="model">The model whose meshes will be used to generate the Buffers</param>
/// <param name="vbos">An insert iterator to a std::pvrvk::Buffer container for the VBOs. Vbos will be inserted using
/// this iterator.</param>
/// <param name="ibos">An insert iterator to an std::pvrvk::Buffer container for the IBOs. Ibos will be inserted using
/// this iterator.</param>
/// <remarks>This utility function will read all vertex data from the VBO. It is usually preferred for meshes
/// meshes containing a single set of interleaved data. If multiple data elements (i.e. sets of interleaved data),
/// each block will be successively placed after the other. The std::inserter this function requires can be created
/// from any container with an insert() function with (for example, for insertion at the end of a vector)
/// std::inserter(std::vector, std::vector::end()) .</remarks>
template<typename VboInsertIterator_, typename IboInsertIterator_>
inline void createSingleBuffersFromModel(pvrvk::Device& device, const assets::Model& model,
    VboInsertIterator_ vbos, IboInsertIterator_ ibos)
{
	createSingleBuffersFromMeshes(device, model.beginMeshes(), model.endMeshes(), vbos, ibos);
}

/// <summary>Auto generates a set of VBOs and a set of IBOs from the vertex data of the meshes of a model and
/// appends them at the end of containers provided by the user.</summary>
/// <param name="device">The device device where the buffers will be generated on</param>
/// <param name="model">The model whose meshes will be used to generate the Buffers</param>
/// <param name="vbos">A container of pvrvk::Buffer handles. The VBOs will be inserted at the end of this
/// container.</param>
/// <param name="ibos">A container of pvrvk::Buffer handles. The IBOs will be inserted at the end of this
/// container.</param>
/// <remarks>This utility function will read all vertex data from the VBO. It is usually preferred for meshes
/// meshes containing a single set of interleaved data. If multiple data elements (i.e. sets of interleaved data),
/// each block will be successively placed after the other.</remarks>
template<typename VboContainer_, typename IboContainer_>
inline void appendSingleBuffersFromModel(pvrvk::Device& device, const assets::Model& model,
    VboContainer_& vbos, IboContainer_& ibos)
{
	createSingleBuffersFromMeshes(device, model.beginMeshes(), model.endMeshes(),
	                              std::inserter(vbos, vbos.end()), std::inserter(ibos, ibos.end()));
}

/// <summary>Creates a 3d plane mesh based on the width and depth specified. Texture coordinates and normal coordinates can also
/// optionally be generated based on the generateTexCoords and generateNormalCoords flags respectively. The generated mesh will be
/// returned as a pvr::assets::Mesh.</summary>
/// <param name="width">The width of the plane to generate.</param>
/// <param name="depth">The depth of the plane to generate.</param>
/// <param name="generateTexCoords">Specifies whether to generate texture coordinates for the plane.</param>
/// <param name="generateNormalCoords">Specifies whether to generate normal coordinates for the plane.</param>
/// <param name="outMesh">The generated pvr::assets::Mesh.</param>
void create3dPlaneMesh(uint32_t width, uint32_t depth, bool generateTexCoords, bool generateNormalCoords, assets::Mesh& outMesh);

/// <summary>Saves the input image as a TGA file with the filename specified. Note that the image must have been created with the
/// VkImageUsageFlags::e_TRANSFER_SRC_BIT set.</summary>
/// <param name="image">The image to save as a TGA file.</param>
/// <param name="imageInitialLayout">The initial layout of the image from which a transition will be made to VkImageLayout::e_TRANSFER_SRC_OPTIMAL.</param>
/// <param name="imageFinalLayout">The final layout of the image to which a transition will be made.</param>
/// <param name="pool">A command pool from which to allocate a temporary command buffer to carry out the transfer operations.</param>
/// <param name="queue">A queue to submit the generated command buffer to. This queue must be compatible with the command pool provided.</param>
/// <param name="filename">The filename to use for the saved TGA image.</param>
/// <param name="screenshotScale">A scaling factor to use for increasing the size of the saved screenshot.</param>
void saveImage(pvrvk::Image image, const VkImageLayout imageInitialLayout, const VkImageLayout imageFinalLayout,
               pvrvk::CommandPool& pool, pvrvk::Queue& queue, const std::string& filename, const uint32_t screenshotScale = 1);

/// <summary>Saves a particular swapchain image corresponding to the swapchain image at index swapIndex for the swapChain.</summary>
/// <param name="swapChain">The swapchain from which a particular image will be saved.</param>
/// <param name="swapIndex">The swapchain image at index swapIndex will be saved as a TGA file.</param>
/// <param name="pool">A command pool from which to allocate a temporary command buffer to carry out the transfer operations.</param>
/// <param name="queue">A queue to submit the generated command buffer to. This queue must be compatible with the command pool provided.</param>
/// <param name="screenshotFileName">The filename to use for the saved TGA image.</param>
/// <param name="screenshotScale">A scaling factor to use for increasing the size of the saved screenshot.</param>
void takeScreenshot(pvrvk::Swapchain& swapChain, const uint32_t swapIndex, pvrvk::CommandPool& pool, pvrvk::Queue& queue,
                    const std::string& screenshotFileName, const uint32_t screenshotScale = 1);

/// <summary>Populate color and depthstencil clear values</summary>
/// <param name="renderpass">The renderpass is used to determine the number of attachments and their formats from which a decision will be as to whether the
/// provided clearColor or clearDepthStencilValue will be used for the corresponding pvrvk::ClearValue structure for each attachment.</param>
/// <param name="clearColor">A pvrvk::ClearValue which will be used as the clear color value for the renderpass attachments with color formats</param>
/// <param name="clearDepthStencilValue">A pvrvk::ClearValue which will be used as the depth stencil value for the renderpass attachments with depth stencil formats</param>
/// <param name="outClearValues">A pointer to an array of pvrvk::ClearValue structures which should have size greater than or equal to the number of renderpass
/// attachments.</param>
inline void populateClearValues(const pvrvk::RenderPass& renderpass, const pvrvk::ClearValue& clearColor, const pvrvk::ClearValue& clearDepthStencilValue, pvrvk::ClearValue* outClearValues)
{
	for (uint32_t i = 0; i < renderpass->getCreateInfo().getNumAttachmentDescription(); ++i)
	{
		const VkFormat& format = renderpass->getCreateInfo().getAttachmentDescription(i).format;
		if (pvrvk::isFormatDepthStencil(format))
		{
			outClearValues[i] = clearDepthStencilValue;
		}
		else
		{
			outClearValues[i] = clearColor;
		}
	}
}

/// <summary>The VulkanVersion structure provides an easy mechanism for constructing the Vulkan version for use when creating a Vulkan instance.</summary>
struct VulkanVersion
{
	/// <summary>The major version number.</summary>
	uint32_t majorV;
	/// <summary>The minor version number.</summary>
	uint32_t minorV;
	/// <summary>The patch version number.</summary>
	uint32_t patchV;

	/// <summary>Default constructor for the VulkanVersion structure initiailising the version to the first Vulkan release 1.0.0.</summary>
	/// <param name="majorV">The major Vulkan version.</param>
	/// <param name="minorV">The minor Vulkan version.</param>
	/// <param name="patchV">The Vulkan patch version.</param>
	VulkanVersion(uint32_t majorV = 1, uint32_t minorV = 0, uint32_t patchV = 0) :
		majorV(majorV), minorV(minorV), patchV(patchV) {}

	/// <summary>Converts the major, minor and patch versions to a uint32_t which can be directly used when creating a Vulkan instance.</summary>
	/// <returns>A uint32_t value which can be directly as the vulkan api version when creating a Vulkan instance set as VkApplicationInfo.apiVersion</returns>
	uint32_t toVulkanVersion() { return VK_MAKE_VERSION(majorV, minorV, patchV); }
};

/// <summary>Utility function for creating a Vulkan instance and pvrvk::Surface using the appropriately set parameters.</summary>
/// <param name="applicationName">Used for setting the pApplicationName of the VkApplicationInfo structure used when calling vkCreateInstance.</param>
/// <param name="window">A pointer to a NativeWindow used to create the windowing surface.</param>
/// <param name="display">A pointer to a NativeDisplay used to create the windowing surface.</param>
/// <param name="outInstance">A reference which will be used as the returned created instance.</param>
/// <param name="outSurface">A reference which will be used as the returned created surface.</param>
/// <param name="apiVersion">A VulkanVersion structure used for setting the apiVersion of the VkApplicationInfo structure used when creating the Vulkan instance.</param>
/// <param name="instanceExtensions">An InstanceExtensions structure which holds a list of instance extensions which will be checked for compatibility with the
/// current Vulkan implementation before setting as the ppEnabledExtensionNames member of the VkInstanceCreateInfo used when creating the Vulkan instance.</param>
/// <param name="layers">An InstanceLayers structure which holds a list of instance layers which will be checked for compatibility with the current Vulkan
/// implementation before setting as the ppEnabledLayerNames member of the VkInstanceCreateInfo used when creating the Vulkan instance.</param>
/// <returns>"true" if the instance and the surface was successfully created.</returns>
bool createInstanceAndSurface(const std::string& applicationName, void* window, void* display, pvrvk::Instance& outInstance,
                              pvrvk::Surface& outSurface, VulkanVersion apiVersion = VulkanVersion(),
                              const InstanceExtensions& instanceExtensions = InstanceExtensions(),
                              const InstanceLayers& layers = InstanceLayers());

/// <summary>Utility function used to determine whether the SurfaceCapabilitiesKHR supportedUsageFlags member contains the specified image usage and therefore can be used in the intended way.</summary>
/// <param name="surfaceCapabilities">A SurfaceCapabilitiesKHR structure returned via a call to PhysicalDevice->getSurfaceCapabilities().</param>
/// <param name="imageUsage">A set of image usage flags which should be checked for support.</param>
/// <returns>"true" if the supportedUsageFlags member of the SurfaceCapabilitiesKHR structure contains the specified imageUsage flag bits.</returns>
inline bool isImageUsageSupportedBySurface(const pvrvk::SurfaceCapabilitiesKHR& surfaceCapabilities, const VkImageUsageFlags& imageUsage)
{
	if (static_cast<uint32_t>(surfaceCapabilities.supportedUsageFlags & imageUsage) != 0)
	{
		return true;
	}
	return false;
}
}
}
