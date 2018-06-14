/*!
\brief The Pipeline Cache class
\file PVRVk/PipelineCacheVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVRVk/DeviceVk.h"
namespace pvrvk {
namespace impl {
/// <summary>A PVRVk pipeline cache object which allows the result of pipeline construction to be reused between pipelines and between runs of an application</summary>
class PipelineCache_ : public DeviceObjectHandle<VkPipelineCache>, public DeviceObjectDebugMarker<PipelineCache_>
{
public:
	/// <summary>Get the maximum size of the data that can be retrieved from the this pipeline cache, in bytes</summary>
	/// <returns>Returns maximum cache size</returns>
	size_t getCacheMaxDataSize() const
	{
		size_t dataSize;
		_device->getVkBindings().vkGetPipelineCacheData(_device->getVkHandle(), getVkHandle(), &dataSize, nullptr);
		return dataSize;
	}

	/// <summary>Get the cache data  and the size</summary>
	/// <param name="size">The size of the buffer, in bytes, pointed to by inOutData</param>
	/// <param name="inOutData">Pointer to the data where it gets written</param>
	/// <returns>Returns the amount of data actually written to the buffer</returns>
	size_t getCacheData(size_t size, void* inOutData) const
	{
		debug_assertion(size && inOutData, "size and the data must be valid");
		size_t mySize = size;
		_device->getVkBindings().vkGetPipelineCacheData(_device->getVkHandle(), getVkHandle(), &mySize, inOutData);
		return mySize;
	}

private:
	template<typename>
	friend struct ::pvrvk::RefCountEntryIntrusive;
	friend class ::pvrvk::impl::Device_;
	DECLARE_NO_COPY_SEMANTICS(PipelineCache_)
	PipelineCache_(DeviceWeakPtr device, size_t initialDataSize, const void* pInitialData, PipelineCacheCreateFlags flags)
		: DeviceObjectHandle(device), DeviceObjectDebugMarker(DebugReportObjectTypeEXT::e_PIPELINE_CACHE_EXT)
	{
		const VkPipelineCacheCreateInfo createInfo{ static_cast<VkStructureType>(StructureType::e_PIPELINE_CACHE_CREATE_INFO), nullptr,
			static_cast<VkPipelineCacheCreateFlags>(flags), initialDataSize, pInitialData };
		vkThrowIfFailed(_device->getVkBindings().vkCreatePipelineCache(_device->getVkHandle(), &createInfo, nullptr, &_vkHandle), "Failed to create Pipeline Cache");
	}

	/// <summary>destructor</summary>
	~PipelineCache_()
	{
		if (_device.isValid())
		{
			_device->getVkBindings().vkDestroyPipelineCache(getDevice()->getVkHandle(), getVkHandle(), nullptr);
			_vkHandle = VK_NULL_HANDLE;
		}
	}
};
} // namespace impl
} // namespace pvrvk
