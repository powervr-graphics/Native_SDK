/*!
\brief Function definitions for the DescriptorSet class.
\file PVRVk/DescriptorSetVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRVk/DescriptorSetVk.h"
#include "PVRVk/ImageVk.h"
#include "PVRVk/SamplerVk.h"
#include "PVRVk/BufferVk.h"
#ifdef DEBUG
#include <algorithm>
#endif

namespace pvrvk {
// DESCRIPTOR POOL
namespace impl {
pvrvk::DescriptorSet DescriptorPool_::allocateDescriptorSet(const DescriptorSetLayout& layout)
{
	DescriptorSet set;
	set.construct(layout, getReference());
	if (!set->init())
	{
		Log("Failed to allocate DescriptorSet");
		set.reset();
	}
	return set;
}

bool DescriptorPool_::init(const DescriptorPoolCreateInfo& createInfo)
{
	VkDescriptorPoolCreateInfo descPoolInfo;
	descPoolInfo.sType = VkStructureType::e_DESCRIPTOR_POOL_CREATE_INFO;
	descPoolInfo.pNext = NULL;
	descPoolInfo.maxSets = createInfo.getMaxDescriptorSets();
	descPoolInfo.flags = VkDescriptorPoolCreateFlags::e_FREE_DESCRIPTOR_SET_BIT;
	VkDescriptorPoolSize poolSize[(int)VkDescriptorType::e_RANGE_SIZE];
	uint32_t poolIndex = 0;
	for (uint32_t i = 0; i < static_cast<uint32_t>(VkDescriptorType::e_RANGE_SIZE); ++i)
	{
		uint32_t count = createInfo.getNumDescriptorTypes(VkDescriptorType(i));
		if (count)
		{
			poolSize[poolIndex].type = (VkDescriptorType)i;
			poolSize[poolIndex].descriptorCount = count;
			++poolIndex;
		}
	}// next type
	descPoolInfo.poolSizeCount = poolIndex;
	descPoolInfo.pPoolSizes = poolSize;

	VkDevice dev = _device->getNativeObject();
	VkResult res = vk::CreateDescriptorPool(dev, &descPoolInfo, nullptr, &_vkDescPool);
	return res == VkResult::e_SUCCESS;
}
void DescriptorPool_::destroy()
{
	if (_vkDescPool != VK_NULL_HANDLE)
	{
		if (_device.isValid())
		{
			vk::DestroyDescriptorPool(_device->getNativeObject(), _vkDescPool, nullptr);
			_vkDescPool = VK_NULL_HANDLE;
			_device.reset();
		}
		else
		{
			reportDestroyedAfterContext("DescriptorPool");
		}
	}
}
}

// DESCRIPTOR SET
namespace impl {
#ifdef DEBUG
static inline bool bindingIdPairComparison(const std::pair<uint32_t, uint32_t>& a,
    const std::pair<uint32_t, uint32_t>& b)
{
	return a.first < b.first;
}
#endif

void DescriptorSet_::destroy()
{
	_keepAlive.clear();
	if (_vkDescriptorSet != VK_NULL_HANDLE)
	{
		if (_descPool->getDevice().isValid())
		{
			vk::FreeDescriptorSets(_descPool->getDevice()->getNativeObject(),
			                       _descPool->getNativeObject(), 1, &_vkDescriptorSet);
			_descPool->getDevice().reset();
		}
		else
		{
			reportDestroyedAfterContext("DescriptorSet");
		}
		_vkDescriptorSet = VK_NULL_HANDLE;
		_descPool.reset();
		_descSetLayout.reset();
	}
}

void DescriptorSetLayout_::destroy()
{
	if (_vkDescsetLayout != VK_NULL_HANDLE)
	{
		if (_device.isValid())
		{
			vk::DestroyDescriptorSetLayout(_device->getNativeObject(), _vkDescsetLayout, nullptr);
			_vkDescsetLayout = VK_NULL_HANDLE;
			_device.reset();
		}
		else
		{
			reportDestroyedAfterContext("DescriptorSetLayout");
		}
	}
	clearCreateInfo();
}

bool DescriptorSetLayout_::init(const DescriptorSetLayoutCreateInfo& createInfo)
{
	_createInfo = createInfo;
	VkDescriptorSetLayoutCreateInfo vkLayoutCreateInfo = {};
	std::vector<VkDescriptorSetLayoutBinding> vkBindings(getCreateInfo().getNumBindings());
	const DescriptorSetLayoutCreateInfo::DescriptorSetLayoutBinding* bindings = createInfo.getAllBindings();
	for (uint32_t i = 0; i < createInfo.getNumBindings(); ++i)
	{
		vkBindings[i].descriptorType = bindings[i].descriptorType;
		vkBindings[i].binding = bindings[i].binding;
		vkBindings[i].descriptorCount = bindings[i].descriptorCount;
		vkBindings[i].stageFlags = bindings[i].stageFlags;
		vkBindings[i].pImmutableSamplers = nullptr;
		if (bindings[i].immutableSampler.isValid())
		{
			vkBindings[i].pImmutableSamplers = &bindings[i].immutableSampler->getNativeObject();
		}
	}
	vkLayoutCreateInfo.sType = VkStructureType::e_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	vkLayoutCreateInfo.bindingCount = (uint32_t)vkBindings.size();
	vkLayoutCreateInfo.pBindings = vkBindings.data();
	return (vk::CreateDescriptorSetLayout(_device->getNativeObject(), &vkLayoutCreateInfo, nullptr, &_vkDescsetLayout) == VkResult::e_SUCCESS);
}
}// namespace impl
}// namespace pvrvk
