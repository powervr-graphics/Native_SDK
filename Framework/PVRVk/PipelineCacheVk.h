#pragma once
#include "PVRVk/DeviceVk.h"
namespace pvrvk
{
namespace impl{
class PipelineCache_
{
public:
	/// <summary>Get the devioce which own this resource </summary>(const)
	/// <returns>DeviceWeakPtr</returns>
	const DeviceWeakPtr& getDevice()const
	{
		return _device;
	}

	/// <summary>Get the devioce which own this resource</summary>
	/// <returns>DeviceWeakPtr</returns>
	DeviceWeakPtr getDevice()
	{
		return _device;
	}

	/// <summary>Get vulkan object</summary>
	/// <returns>VkPipelineCache</returns>
	const VkPipelineCache& getNativeObject()const
	{
		return _vkCache;
	}

	/// <summary>Get the maximum size of the data that can be retrieved from the this pipeline cache, in bytes</summary>
	/// <returns>Returns maximum cache size</returns>
	size_t getCacheMaxDataSize()const
	{
		size_t dataSize;
		vk::GetPipelineCacheData(_device->getNativeObject(), _vkCache, &dataSize, nullptr);
		return dataSize;
	}

	/// <summary>Get the cache data  and the size</summary>
	/// <param name="size">The size of the buffer, in bytes, pointed to by inOutData</param>
	/// <param name="inOutData">Pointer to the data where it gets written</param>
	/// <returns>Returns the amount of data actually written to the buffer</returns>
	size_t getCacheData(size_t size, void* inOutData)const
	{
		debug_assertion(size && inOutData, "size and the data must be valid");
		size_t mySize = size;
		vk::GetPipelineCacheData(_device->getNativeObject(), _vkCache, &mySize, inOutData);
		return mySize;
	}

private:
	template<typename> friend struct ::pvrvk::RefCountEntryIntrusive;
	friend class ::pvrvk::impl::Device_;
	DECLARE_NO_COPY_SEMANTICS(PipelineCache_)
	PipelineCache_(DeviceWeakPtr device) : _device(device), _vkCache(VK_NULL_HANDLE){}

	VkResult init(size_t initialDataSize, const void* pInitialData, VkPipelineCacheCreateFlags flags)
	{
		const VkPipelineCacheCreateInfo   createInfo
		{
			VkStructureType::e_PIPELINE_CACHE_CREATE_INFO,
			0,
			flags,
			initialDataSize,
			pInitialData
		};
		return vk::CreatePipelineCache(_device->getNativeObject(), &createInfo, nullptr, &_vkCache);
	}

	/// <summary>destructor</summary>
	~PipelineCache_()
	{
		if(_device.isValid())
		{
			vk::DestroyPipelineCache(getDevice()->getNativeObject(), _vkCache, nullptr);
			_vkCache = VK_NULL_HANDLE;
		}
	}
	VkPipelineCache         _vkCache;
	DeviceWeakPtr           _device;
};

}
}
