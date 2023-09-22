/*!
\brief PVRVk Semaphore class.
\file PVRVk/SemaphoreVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRVk/DeviceVk.h"

namespace pvrvk {
#if defined(_WIN32)
#undef MemoryBarrier
#endif
	
/// <summary>A Global memory barrier used for memory accesses for all memory objects.</summary>
template<class AccessFlagsType> class MemoryBarrierTemplate
{
protected:
	AccessFlagsType srcAccessMask; //!< Bitmask of pvrvk::AccessFlagBits specifying a source access mask.
	AccessFlagsType dstAccessMask; //!< Bitmask of pvrvk::AccessFlagBits specifying a destination access mask.

public:
	/// <summary>Constructor, zero initialization</summary>
	MemoryBarrierTemplate() : srcAccessMask(AccessFlagsType(0)), dstAccessMask(AccessFlagsType(0)) {}

	/// <summary>Constructor, setting all members</summary>
	/// <param name="srcAccessMaskParam">Bitmask of pvrvk::AccessFlagBits specifying a source access mask.</param>
	/// <param name="dstAccessMaskParam">Bitmask of pvrvk::AccessFlagBits specifying a destination access mask.</param>
	MemoryBarrierTemplate(AccessFlagsType srcAccessMaskParam, AccessFlagsType dstAccessMaskParam) : srcAccessMask(srcAccessMaskParam), dstAccessMask(dstAccessMaskParam) {}

	/// <summary>Get srcAccessMask</summary>
	/// <returns>An AccessFlagsType structure specifying the source memory barrier access flags</returns>
	inline const AccessFlagsType& getSrcAccessMask() const { return srcAccessMask; }

	/// <summary>Set srcAccessMask</summary>
	/// <param name="inSrcAccessMask">An AccessFlagsType structure specifying the source memory barrier access flags</param>
	inline void setSrcAccessMask(const AccessFlagsType& inSrcAccessMask) { this->srcAccessMask = inSrcAccessMask; }

	/// <summary>Get dstAccessMask</summary>
	/// <returns>An AccessFlagsType structure specifying the destination memory barrier access flags</returns>
	inline const AccessFlagsType& getDstAccessMask() const { return dstAccessMask; }

	/// <summary>Set dstAccessMask</summary>
	/// <param name="inDstAccessMask">An AccessFlagsType structure specifying the destination memory barrier access flags</param>
	inline void setDstAccessMask(const AccessFlagsType& inDstAccessMask) { this->dstAccessMask = inDstAccessMask; }
};


/// <summary>A Buffer memory barrier used only for memory accesses involving a specific range of the specified
/// buffer object. It is also used to transfer ownership of an buffer range from one queue family to another.</summary>
//class BufferMemoryBarrier : public MemoryBarrier
template<class AccessFlagsType> class BufferMemoryBarrierTemplate: public MemoryBarrierTemplate<AccessFlagsType>
{
protected:
	Buffer buffer; //!< Handle to the buffer whose backing memory is affected by the barrier.
	uint32_t offset; //!< Offset in bytes into the backing memory for buffer. This is relative to the base offset as bound to the buffer
	uint32_t size; //!< Size in bytes of the affected area of backing memory for buffer, or VK_WHOLE_SIZE to use the range from offset to the end of the buffer.

public:
	/// <summary>Constructor, zero initialization</summary>
	BufferMemoryBarrierTemplate()
	{}

	/// <summary>Constructor, individual elements</summary>
	/// <param name="srcAccessMask">Bitmask of pvrvk::AccessFlagBits specifying a source access mask.</param>
	/// <param name="dstAccessMask">Bitmask of pvrvk::AccessFlagBits specifying a destination access mask.</param>
	/// <param name="bufferParam">Handle to the buffer whose backing memory is affected by the barrier.</param>
	/// <param name="offsetParam">Offset in bytes into the backing memory for buffer. This is relative to the base offset as bound to the buffer</param>
	/// <param name="sizeParam">Size in bytes of the affected area of backing memory for buffer, or VK_WHOLE_SIZE to use the range from offset to the end of the buffer.</param>
	BufferMemoryBarrierTemplate(AccessFlagsType srcAccessMaskParam, AccessFlagsType dstAccessMaskParam, Buffer bufferParam, uint32_t offsetParam, uint32_t sizeParam)
		: MemoryBarrierTemplate<AccessFlagsType>(srcAccessMaskParam, dstAccessMaskParam), buffer(bufferParam), offset(offsetParam), size(sizeParam)
	{}

	/// <summary>Get Buffer associated with the memory barrier</summary>
	/// <returns>The PVRVk::Buffer associated with the memory barrier</returns>
	inline const Buffer& getBuffer() const { return buffer; }

	/// <summary>Set buffer associated with the memory barrier</summary>
	/// <param name="inBuffer">The PVRVk::Buffer associated with the memory barrier</param>
	inline void setBuffer(const Buffer& inBuffer) { this->buffer = inBuffer; }

	/// <summary>Get size corresponding to the slice of the Buffer associated with the memory barrier</summary>
	/// <returns>The size of the range of the PVRVk::Buffer associated with the memory barrier</returns>
	inline uint32_t getSize() const { return size; }

	/// <summary>Set the size of the slice of the buffer associated with the memory barrier</summary>
	/// <param name="inSize">The size of the slice of the PVRVk::Buffer associated with the memory barrier</param>
	inline void setSize(uint32_t inSize) { this->size = inSize; }

	/// <summary>Get the offset into the Buffer associated with the memory barrier</summary>
	/// <returns>The offset into PVRVk::Buffer associated with the memory barrier</returns>
	inline uint32_t getOffset() const { return offset; }

	/// <summary>Set the offset into the buffer associated with the memory barrier</summary>
	/// <param name="inOffset">The offset into the PVRVk::Buffer associated with the memory barrier</param>
	inline void setOffset(uint32_t inOffset) { this->offset = inOffset; }
};


class BarrierPipelineStageFlag2
{
protected:
	pvrvk::PipelineStageFlagBits2KHR srcStageMask; //!< Bitmask for the pipeline stage source stage mask, which is per-barrier in VK_KHR_synchronization2
	pvrvk::PipelineStageFlagBits2KHR dstStageMask; //!< Bitmask for the pipeline stage destination stage mask, which is per-barrier in VK_KHR_synchronization2

public:
	/// <summary>Constructor, zero initialization</summary>
	BarrierPipelineStageFlag2() : srcStageMask(static_cast<pvrvk::PipelineStageFlagBits2KHR>(0)), dstStageMask(static_cast<pvrvk::PipelineStageFlagBits2KHR>(0)) {}

	/// <summary>Constructor, individual elements</summary>
	/// <param name="srcStageMaskParam">Bitmask of pvrvk::PipelineStageFlagBits2KHR specifying a source pipeline stage mask.</param>
	/// <param name="dstStageMaskParam">Bitmask of pvrvk::PipelineStageFlagBits2KHR specifying a destination pipeline stage mask.</param>
	BarrierPipelineStageFlag2(pvrvk::PipelineStageFlagBits2KHR srcStageMaskParam, pvrvk::PipelineStageFlagBits2KHR dstStageMaskParam)
		: srcStageMask(srcStageMaskParam), dstStageMask(dstStageMaskParam)
	{}

	/// <summary>Get srcStageMask</summary>
	/// <returns>An PipelineStageFlagBits2KHR structure specifying the source pipeline stage flags</returns>
	inline const PipelineStageFlagBits2KHR& getSrcStageMask() const { return srcStageMask; }

	/// <summary>Set srcStageMask</summary>
	/// <param name="inSrcStageMask">An PipelineStageFlagBits2KHR structure specifying the source pipeline stage flags</param>
	inline void setSrcStageMask(const PipelineStageFlagBits2KHR& inSrcStageMask) { this->srcStageMask = inSrcStageMask; }

	/// <summary>Get dstStageMask</summary>
	/// <returns>An PipelineStageFlagBits2KHR structure specifying the destination pipeline stage flags</returns>
	inline const PipelineStageFlagBits2KHR& getDstStageMask() const { return dstStageMask; }

	/// <summary>Set dstStageMask</summary>
	/// <param name="inDstStageMask">An PipelineStageFlagBits2KHR structure specifying the destination pipeline stage flags</param>
	inline void setDstStageMask(const PipelineStageFlagBits2KHR& inDstStageMask) { this->dstStageMask = inDstStageMask; }
};

class BarrierQueueFamilyIndex
{
protected:
	uint32_t srcQueueFamilyIndex; //!< Source queue family for a queue family ownership transfer.
	uint32_t dstQueueFamilyIndex; //!< Destination queue family for a queue family ownership transfer

public:
	/// <summary>Constructor, zero initialization</summary>
	BarrierQueueFamilyIndex() : srcQueueFamilyIndex(0), dstQueueFamilyIndex(0) {}

	/// <summary>Constructor, individual elements</summary>
	/// <param name="srcQueueFamilyIndexParam">Source queue index.</param>
	/// <param name="dstQueueFamilyIndexParam">Destination queue index.</param>
	BarrierQueueFamilyIndex(uint32_t srcQueueFamilyIndexParam, uint32_t dstQueueFamilyIndexParam)
		: srcQueueFamilyIndex(srcQueueFamilyIndexParam), dstQueueFamilyIndex(dstQueueFamilyIndexParam)
	{}

	/// <summary>Get the source queue family index for the image associated with the memory barrier</summary>
	/// <returns>The source queue family index of the image associated with the memory barrier</returns>
	inline uint32_t getSrcQueueFamilyIndex() const { return srcQueueFamilyIndex; }

	/// <summary>Set the source queue family index</summary>
	/// <param name="inSrcQueueFamilyIndex">The source queue family index of the image associated with the memory barrier</param>
	inline void setSrcQueueFamilyIndex(uint32_t inSrcQueueFamilyIndex) { this->srcQueueFamilyIndex = inSrcQueueFamilyIndex; }

	/// <summary>Get the destination queue family index for the image associated with the memory barrier</summary>
	/// <returns>The destination queue family index of the image associated with the memory barrier</returns>
	inline uint32_t getDstQueueFamilyIndex() const { return dstQueueFamilyIndex; }

	/// <summary>Set the destination queue family index</summary>
	/// <param name="inDstQueueFamilyIndex">The destination queue family index of the image associated with the memory barrier</param>
	inline void setDstQueueFamilyIndex(uint32_t inDstQueueFamilyIndex) { this->dstQueueFamilyIndex = inDstQueueFamilyIndex; }
};


/// <summary>A Image memory barrier used only for memory accesses involving a specific subresource range of the
/// specified image object. It is also used to perform a layout transition for an image subresource range, or to
/// transfer ownership of an image subresource range from one queue family to another.</summary>
template<class AccessFlagsType> class ImageMemoryBarrierTemplate : public MemoryBarrierTemplate<AccessFlagsType>, public BarrierQueueFamilyIndex
{
protected:
	pvrvk::ImageLayout oldLayout; //!< Old layout in an image layout transition.
	pvrvk::ImageLayout newLayout; //!< New layout in an image layout transition.
	Image image; //!< Handle to the image affected by this barrier
	ImageSubresourceRange subresourceRange; //!< Describes the image subresource range within image that is affected by this barrier

public:
	/// <summary>Constructor. All flags are zero initialized, and family indexes set to -1.</summary>
	ImageMemoryBarrierTemplate()
		: oldLayout(pvrvk::ImageLayout::e_UNDEFINED), newLayout(pvrvk::ImageLayout::e_UNDEFINED)
	{}

	/// <summary>Constructor. Set all individual elements.</summary>
	/// <param name="srcMask">Bitmask of pvrvk::AccessFlagBits specifying a source access mask.</param>
	/// <param name="dstMask">Bitmask of pvrvk::AccessFlagBits specifying a destination access mask.</param>
	/// <param name="imageParam">Handle to the image affected by this barrier</param>
	/// <param name="subresourceRangeParam">Describes the image subresource range within image that is affected by this barrier</param>
	/// <param name="oldLayoutParam">Old layout in an image layout transition.</param>
	/// <param name="newLayoutParam">New layout in an image layout transition.</param>
	/// <param name="srcQueueFamilyIndexParam">Source queue family for a queue family ownership transfer.</param>
	/// <param name="dstQueueFamilyIndexParam">Destination queue family for a queue family ownership transfer</param>
	ImageMemoryBarrierTemplate(AccessFlagsType srcMask, AccessFlagsType dstMask, const Image& imageParam, const ImageSubresourceRange& subresourceRangeParam,
		pvrvk::ImageLayout oldLayoutParam, pvrvk::ImageLayout newLayoutParam, uint32_t srcQueueFamilyIndexParam, uint32_t dstQueueFamilyIndexParam)
		: MemoryBarrierTemplate<AccessFlagsType>(srcMask, dstMask), 
		BarrierQueueFamilyIndex(srcQueueFamilyIndexParam, dstQueueFamilyIndexParam),
		oldLayout(oldLayoutParam), newLayout(newLayoutParam), 
		image(imageParam), subresourceRange(subresourceRangeParam)
	{}

	/// <summary>Get the old image layout of the image associated with the memory barrier</summary>
	/// <returns>The old image layout of the image associated with the memory barrier</returns>
	inline const ImageLayout& getOldLayout() const { return oldLayout; }

	/// <summary>Set old image layout</summary>
	/// <param name="inOldLayout">The old image layout of the image associated memory barrier</param>
	inline void setOldLayout(const ImageLayout& inOldLayout) { this->oldLayout = inOldLayout; }

	/// <summary>Get the new image layout of the image associated with the memory barrier</summary>
	/// <returns>The new image layout of the image associated with the memory barrier</returns>
	inline const ImageLayout& getNewLayout() const { return newLayout; }

	/// <summary>Set new image layout</summary>
	/// <param name="inNewLayout">The new image layout of the image associated memory barrier</param>
	inline void setNewLayout(const ImageLayout& inNewLayout) { this->newLayout = inNewLayout; }

	/// <summary>Get Image</summary>
	/// <returns>The PVRVk::Image associated with the memory barrier</returns>
	inline const Image& getImage() const { return image; }

	/// <summary>Set Image</summary>
	/// <param name="inImage">The PVRVk::Image associated with the memory barrier</param>
	inline void setImage(const Image& inImage) { this->image = inImage; }

	/// <summary>Get the subresource range of the image associated with the memory barrier</summary>
	/// <returns>The subresource range of the image associated with the memory barrier</returns>
	inline const ImageSubresourceRange& getSubresourceRange() const { return subresourceRange; }

	/// <summary>Set the subresource range of the image associated with the memory barrier</summary>
	/// <param name="inSubresourceRange">The subresource range of the image associated with the memory barrier</param>
	inline void setSubresourceRange(const ImageSubresourceRange& inSubresourceRange) { this->subresourceRange = inSubresourceRange; }
};


/// <summary>A memory barrier similar to MemoryBarrier but used in VK_KHR_synchronization2.</summary>
class MemoryBarrier2 : public MemoryBarrierTemplate<pvrvk::AccessFlagBits2KHR>, public BarrierPipelineStageFlag2
{
public:
	/// <summary>Constructor, zero initialization</summary>
	MemoryBarrier2() {}

	/// <summary>Constructor, individual elements</summary>
	/// <param name="srcStageMask">Bitmask for the source pipeline stage as in VK_KHR_synchronization2 it is defined per barrier.</param>
	/// <param name="dstStageMask">Bitmask for the destination pipeline stage as in VK_KHR_synchronization2 it is defined per barrier.</param>
	/// <param name="srcAccessMask">Bitmask of pvrvk::AccessFlagBits specifying a source access mask.</param>
	/// <param name="dstAccessMask">Bitmask of pvrvk::AccessFlagBits specifying a destination access mask.</param>
	MemoryBarrier2(pvrvk::PipelineStageFlagBits2KHR srcStageMask, pvrvk::PipelineStageFlagBits2KHR dstStageMask, pvrvk::AccessFlagBits2KHR srcAccessMask,
		pvrvk::AccessFlagBits2KHR dstAccessMask)
		: MemoryBarrierTemplate<pvrvk::AccessFlagBits2KHR>(srcAccessMask, dstAccessMask), BarrierPipelineStageFlag2(srcStageMask, dstStageMask)
	{}
};

/// <summary>A Buffer memory barrier similar to BufferMemoryBarrier but used in VK_KHR_synchronization2.</summary>
class BufferMemoryBarrier2 : public BufferMemoryBarrierTemplate<pvrvk::AccessFlagBits2KHR>, public BarrierPipelineStageFlag2, public BarrierQueueFamilyIndex
{
public:
	/// <summary>Constructor, zero initialization</summary>
	BufferMemoryBarrier2() {}

	/// <summary>Constructor, individual elements</summary>
	/// <param name="srcStageMask">Bitmask for the source pipeline stage as in VK_KHR_synchronization2 it is defined per barrier.</param>
	/// <param name="dstStageMask">Bitmask for the destination pipeline stage as in VK_KHR_synchronization2 it is defined per barrier.</param>
	/// <param name="srcAccessMask">Bitmask of pvrvk::AccessFlagBits specifying a source access mask.</param>
	/// <param name="dstAccessMask">Bitmask of pvrvk::AccessFlagBits specifying a destination access mask.</param>
	/// <param name="srcQueueFamilyIndexParam">Source queue family for a queue family ownership transfer.</param>
	/// <param name="dstQueueFamilyIndexParam">Destination queue family for a queue family ownership transfer</param>
	/// <param name="buffer">Handle to the buffer whose backing memory is affected by the barrier.</param>
	/// <param name="offset">Offset in bytes into the backing memory for buffer. This is relative to the base offset as bound to the buffer</param>
	/// <param name="size">Size in bytes of the affected area of backing memory for buffer, or VK_WHOLE_SIZE to use the range from offset to the end of the buffer.</param>
	BufferMemoryBarrier2(pvrvk::PipelineStageFlagBits2KHR srcStageMask, pvrvk::PipelineStageFlagBits2KHR dstStageMask, pvrvk::AccessFlagBits2KHR srcAccessMask,
		pvrvk::AccessFlagBits2KHR dstAccessMask, uint32_t srcQueueFamilyIndexParam, uint32_t dstQueueFamilyIndexParam, Buffer buffer, uint32_t offset, uint32_t size)
		: BufferMemoryBarrierTemplate(srcAccessMask, dstAccessMask, buffer, offset, size), 
		  BarrierPipelineStageFlag2(srcStageMask, dstStageMask),
		  BarrierQueueFamilyIndex(srcQueueFamilyIndexParam, dstQueueFamilyIndexParam)
	{}
};


/// <summary>A Buffer memory barrier similar to ImageMemoryBarrier but used in VK_KHR_synchronization2.</summary>
class ImageMemoryBarrier2 : public ImageMemoryBarrierTemplate<pvrvk::AccessFlagBits2KHR>, public BarrierPipelineStageFlag2
{
public:
	/// <summary>Constructor. All flags are zero initialized, and family indexes set to -1.</summary>
	ImageMemoryBarrier2()
	{}

	/// <summary>Constructor. Set all individual elements.</summary>
	/// <param name="srcStageMask">Bitmask for the source pipeline stage as in VK_KHR_synchronization2 it is defined per barrier.</param>
	/// <param name="dstStageMask">Bitmask for the destination pipeline stage as in VK_KHR_synchronization2 it is defined per barrier.</param>
	/// <param name="srcMask">Bitmask of pvrvk::AccessFlagBits specifying a source access mask.</param>
	/// <param name="dstMask">Bitmask of pvrvk::AccessFlagBits specifying a destination access mask.</param>
	/// <param name="image">Handle to the image affected by this barrier</param>
	/// <param name="subresourceRange">Describes the image subresource range within image that is affected by this barrier</param>
	/// <param name="oldLayout">Old layout in an image layout transition.</param>
	/// <param name="newLayout">New layout in an image layout transition.</param>
	/// <param name="srcQueueFamilyIndex">Source queue family for a queue family ownership transfer.</param>
	/// <param name="dstQueueFamilyIndex">Destination queue family for a queue family ownership transfer</param>
	ImageMemoryBarrier2(pvrvk::PipelineStageFlagBits2KHR srcStageMask, pvrvk::PipelineStageFlagBits2KHR dstStageMask, pvrvk::AccessFlagBits2KHR srcMask,
		pvrvk::AccessFlagBits2KHR dstMask,
		const Image& image, const ImageSubresourceRange& subresourceRange,
		pvrvk::ImageLayout oldLayout, pvrvk::ImageLayout newLayout, uint32_t srcQueueFamilyIndex, uint32_t dstQueueFamilyIndex)
		: ImageMemoryBarrierTemplate<pvrvk::AccessFlagBits2KHR>(srcMask, dstMask, image, subresourceRange, oldLayout, newLayout, srcQueueFamilyIndex, dstQueueFamilyIndex),
		  BarrierPipelineStageFlag2(srcStageMask, dstStageMask)
	{}
};


typedef MemoryBarrierTemplate<pvrvk::AccessFlags> MemoryBarrier;
typedef BufferMemoryBarrierTemplate<pvrvk::AccessFlags> BufferMemoryBarrier;
typedef ImageMemoryBarrierTemplate<pvrvk::AccessFlags> ImageMemoryBarrier;


/// <summary>Templatized utility function to hold all the memory, barrier and image barriers for both usual and VK_KHR_synchronization2 structs.</summary>
template<class MemoryBarrierType, class BufferMemoryBarrierType, class ImageMemoryBarrierType>
class MemoryBarrierSetTemplate
{
private:
	MemoryBarrierSetTemplate& operator=(const MemoryBarrierSetTemplate&) = delete; // deleted

	std::vector<MemoryBarrierType> memBarriers;
	std::vector<ImageMemoryBarrierType> imageBarriers;
	std::vector<BufferMemoryBarrierType> bufferBarriers;

public:
	/// <summary>Constructor. Empty barrier</summary>
	MemoryBarrierSetTemplate() = default;

	/// <summary>Clear this object of all barriers</summary>
	/// <returns>MemoryBarrierSetTemplate&</returns>
	MemoryBarrierSetTemplate& clearAllBarriers()
	{
		memBarriers.clear();
		imageBarriers.clear();
		bufferBarriers.clear();
		return *this;
	}

	/// <summary>Clear this object of all Memory barriers</summary>
	/// <returns>MemoryBarrierSetTemplate&</returns>
	MemoryBarrierSetTemplate& clearAllMemoryBarriers()
	{
		memBarriers.clear();
		return *this;
	}

	/// <summary>Clear this object of all Buffer barriers</summary>
	/// <returns>MemoryBarrierSetTemplate&</returns>
	MemoryBarrierSetTemplate& clearAllBufferRangeBarriers()
	{
		bufferBarriers.clear();
		return *this;
	}

	/// <summary>Clear this object of all Image barriers</summary>
	/// <returns>MemoryBarrierSetTemplate&</returns>
	MemoryBarrierSetTemplate& clearAllImageAreaBarriers()
	{
		imageBarriers.clear();
		return *this;
	}

	/// <summary>Add a generic Memory barrier.</summary>
	/// <param name="barrier">The barrier to add</param>
	/// <returns>This object (allow chained calls)</returns>
	MemoryBarrierSetTemplate& addBarrier(MemoryBarrierType barrier)
	{
		memBarriers.emplace_back(barrier);
		return *this;
	}

	/// <summary>Add a Buffer Range barrier, signifying that operations on a part of a buffer
	/// must complete before other operations on that part of the buffer execute.</summary>
	/// <param name="barrier">The barrier to add</param>
	/// <returns>This object (allow chained calls)</returns>
	MemoryBarrierSetTemplate& addBarrier(const BufferMemoryBarrierType& barrier)
	{
		bufferBarriers.emplace_back(barrier);
		return *this;
	}

	/// <summary>Add a Buffer Range barrier, signifying that operations on a part of an Image
	/// must complete before other operations on that part of the Image execute.</summary>
	/// <param name="barrier">The barrier to add</param>
	/// <returns>This object (allow chained calls)</returns>
	MemoryBarrierSetTemplate& addBarrier(const ImageMemoryBarrierType& barrier)
	{
		imageBarriers.emplace_back(barrier);
		return *this;
	}

	/// <summary>Get an array of the MemoryBarrierType object of this set.</summary>
	/// <returns>All MemoryBarrierType objects that this object contains</returns>
	const std::vector<MemoryBarrierType>& getMemoryBarriers() const { return this->memBarriers; }

	/// <summary>Get an array of the Image Barriers of this set.</summary>
	/// <returns>All MemoryBarrierType objects that this object contains</returns>
	const std::vector<ImageMemoryBarrierType>& getImageBarriers() const { return this->imageBarriers; }

	/// <summary>Get an array of the Buffer Barriers of this set.</summary>
	/// <returns>All MemoryBarrierType objects that this object contains</returns>
	const std::vector<BufferMemoryBarrierType>& getBufferBarriers() const { return this->bufferBarriers; }
};

typedef MemoryBarrierSetTemplate<MemoryBarrier, BufferMemoryBarrier, ImageMemoryBarrier> MemoryBarrierSet;
typedef MemoryBarrierSetTemplate<MemoryBarrier2, BufferMemoryBarrier2, ImageMemoryBarrier2> MemoryBarrierSet2;

} // namespace pvrvk
