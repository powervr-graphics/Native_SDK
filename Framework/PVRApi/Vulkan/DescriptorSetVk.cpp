/*!*********************************************************************************************************************
\file         PVRApi\Vulkan\DescriptorSetVk.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Vulkan implementation of the DescriptorPool class.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRApi/Vulkan/DescriptorSetVk.h"
#include "PVRApi/Vulkan/TextureVk.h"
#include "PVRApi/Vulkan/SamplerVk.h"
#include "PVRApi/Vulkan/BufferVk.h"

namespace pvr {
namespace api {
namespace impl {
api::DescriptorSet DescriptorPool_::allocateDescriptorSet(const DescriptorSetLayout& layout)
{
	DescriptorPool this_ref = static_cast<vulkan::DescriptorPoolVk_*>(this)->getReference();

	api::vulkan::DescriptorSetVk set;
	set.construct(layout, this_ref);
	if (!set->init())
	{
		set.reset();
	}
	return set;
}
bool DescriptorSet_::update(const DescriptorSetUpdate& descSet)
{
	return native_cast(*this).update(descSet);
}

const native::HDescriptorSet_& impl::DescriptorSet_::getNativeObject() const
{
	return native_cast(*this);
}

native::HDescriptorSet_& impl::DescriptorSet_::getNativeObject()
{
	return native_cast(*this);
}

const native::HDescriptorPool_& DescriptorPool_::getNativeObject()const
{
	return native_cast(*this);
}

native::HDescriptorPool_& DescriptorPool_::getNativeObject()
{
	return native_cast(*this);
}

}

namespace vulkan {

void DescriptorSetVk_::destroy()
{
	auto& vkobj = native_cast(*this);
	if (vkobj.handle != VK_NULL_HANDLE)
	{
		if (m_descPool->getContext().isValid())
		{
			vk::FreeDescriptorSets(native_cast(*getContext()).getDevice(), native_cast(m_descPool)->handle, 1, &native_cast(*this).handle);
		}
		vkobj.handle = VK_NULL_HANDLE;
		m_descPool.reset();
		m_descSetLayout.reset();
	}
}
void DescriptorSetLayoutVk_::destroy()
{

	if (device.isValid())
	{
		if (handle != VK_NULL_HANDLE)
		{
			vk::DestroyDescriptorSetLayout(native_cast(*getContext()).getDevice(), native_cast(*this), NULL);
		}
		device.reset();
	}
	desc.clear();
}

DescriptorSetVk_::~DescriptorSetVk_()
{
	if (m_descPool.isValid() )
	{
		if (m_descPool->getContext().isValid())
		{
			destroy();
		}
		else
		{
			Log(Log.Warning, "Attempted to free DescriptorSet after its corresponding device was destroyed");
		}
	}
	else
	{
		Log(Log.Warning, "Attempted to free DescriptorSet after its corresponding pool was destroyed");
	}
}

//--- DescriptorSet Implementation
bool DescriptorSetVk_::update(const DescriptorSetUpdate& descSet)
{
	std::vector<VkDescriptorImageInfo> imageInfos;
	std::vector<VkDescriptorBufferInfo> bufferInfo;
	std::vector<VkBufferView> unusedInfo;
	std::vector<VkWriteDescriptorSet> descSetWritesVk;

	///// ASSUMING EVERYTHING IS PROPERLY SORTED!!! //////

	// update the combined image sampler
	if (descSet.getBindingList().combinedSamplerImage.size())
	{
		imageInfos.resize(descSet.getBindingList().combinedSamplerImage.size());

		// Current Implementation assumes that array indexes are linear, and they are in the order of binding id.
		// bindingId : arrayIndex
		// 0:0, 0:1, 0:2, 1:0, 1:1, 1:2, ....
		VkWriteDescriptorSet tempDescSet;
		memset(&tempDescSet, 0, sizeof(tempDescSet));
		tempDescSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		tempDescSet.pNext = NULL;

		int32 lastBindingId = -1;
		uint32 lastArrayIndex = (uint32) - 1;
		uint32 imageInfosArrayOffset = 0;

		// for each descriptor
		// - Validate the buffer binding id and the array index are linear.
		// - add a new descriptor set if its a new binding or end of array of the binding
		for (pvr::uint32 i = 0; i < descSet.getBindingList().combinedSamplerImage.size(); ++i)
		{
			auto& bindingInfo = descSet.getBindingList().combinedSamplerImage[i];
			if (bindingInfo.bindingId < 0) { ++imageInfosArrayOffset; continue; }

			assertion(i == 0 || bindingInfo.bindingId != lastBindingId || bindingInfo.arrayIndex == lastArrayIndex + 1);

			DescriptorSetUpdate::CombinedImageSampler binding = bindingInfo.binding;
			imageInfos[i].imageView = native_cast(*binding.second).handle;
			imageInfos[i].sampler = native_cast(*binding.first).handle;
			imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			if (bindingInfo.bindingId != lastBindingId) // if its a new binding or end of array of the binding
			{
				tempDescSet.descriptorCount = 0;
				tempDescSet.dstBinding = lastBindingId = bindingInfo.bindingId;
				tempDescSet.dstArrayElement = lastArrayIndex = bindingInfo.arrayIndex;
				tempDescSet.dstSet = handle;
				tempDescSet.pImageInfo = imageInfos.data() + imageInfosArrayOffset;
				tempDescSet.descriptorType = ConvertToVk::descriptorType(bindingInfo.type);;
				descSetWritesVk.push_back(tempDescSet);
			}

			++imageInfosArrayOffset;// advance the offset.
			descSetWritesVk.back().descriptorCount++;
		}
	}

	uint32 bufferInfosArrayOffset = 0;
	bufferInfo.resize(descSet.getBindingList().buffers.size());
	// ubos
	if (descSet.getBindingList().buffers.size())
	{

		// Current Implementation assumes that array indexes are linear, and they are in the order of binding id.
		// bindingId : arrayIndex
		// 0:0, 0:1, 0:2, 1:0, 1:1, 1:2, ....
		VkWriteDescriptorSet tempDescSet;
		memset(&tempDescSet, 0, sizeof(tempDescSet));
		tempDescSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		tempDescSet.pNext = NULL;

		int32 lastBindingId = -1;
		uint32 lastArrayIndex = (uint32) - 1;

		// for each descriptor
		// - Validate the buffer binding id and the array index are linear.
		// - add a new descriptor set if its a new binding or end of array of the binding
		for (pvr::uint32 i = 0; i < descSet.getBindingList().buffers.size(); ++i)
		{
			auto& bindingInfo = descSet.getBindingList().buffers[i];
			if (!bindingInfo.binding.isValid())
			{
				++bufferInfosArrayOffset;
				continue;
			}
			assertion(i == 0 || bindingInfo.bindingId != lastBindingId || bindingInfo.arrayIndex == lastArrayIndex + 1);

			auto binding = bindingInfo.binding;
			bufferInfo[i].buffer = native_cast(*binding->getResource()).buffer;
			bufferInfo[i].offset = binding->getOffset();
			bufferInfo[i].range = binding->getRange();
			if (bindingInfo.bindingId != lastBindingId) // if its a new binding or end of array of the binding
			{
				tempDescSet.descriptorCount = 0;
				tempDescSet.dstBinding = lastBindingId = bindingInfo.bindingId;
				tempDescSet.dstArrayElement = lastArrayIndex = bindingInfo.arrayIndex;
				tempDescSet.dstSet = handle;
				tempDescSet.pBufferInfo = &bufferInfo[bufferInfosArrayOffset];
				tempDescSet.descriptorType = ConvertToVk::descriptorType(bindingInfo.type);
				descSetWritesVk.push_back(tempDescSet);
			}
			++bufferInfosArrayOffset;// advance the offset.
			++descSetWritesVk.back().descriptorCount;
		}
	}

	if (descSetWritesVk.size())
	{
		vk::UpdateDescriptorSets(native_cast(*m_descSetLayout->getContext()).getDevice(), (uint32)descSetWritesVk.size(), descSetWritesVk.data(), 0, 0);
	}
	return true;
}

//--- DescriptorPool Implementation
bool DescriptorPoolVk_::init(const DescriptorPoolCreateParam& createParam)
{
	VkDescriptorPoolCreateInfo descPoolInfo;
	descPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descPoolInfo.pNext = NULL;
	descPoolInfo.maxSets = createParam.getMaxSets();
	descPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	VkDescriptorPoolSize poolSize[types::DescriptorType::Count];
	pvr::uint32 i = 0, poolIndex = 0;
	for (; i < types::DescriptorType::Count; ++i)
	{
		pvr::uint32 count = createParam.getDescriptorTypeCount(static_cast<types::DescriptorType::Enum>(i));
		if (count)
		{
			poolSize[poolIndex].type = ConvertToVk::descriptorType(static_cast<types::DescriptorType::Enum>(i));
			poolSize[poolIndex].descriptorCount = count;
			++poolIndex;
		}
	}// next type
	descPoolInfo.poolSizeCount = poolIndex;
	descPoolInfo.pPoolSizes = poolSize;

	VkDevice dev = native_cast(*m_context).getDevice();
	VkDescriptorPool pool;
	VkResult res = vk::CreateDescriptorPool(dev, &descPoolInfo, NULL, &pool);
	native_cast(this)->handle = pool;
	return res == VK_SUCCESS;
}
void DescriptorPoolVk_::destroy()
{
	if (handle)
	{
		vk::DestroyDescriptorPool(native_cast(*m_context).getDevice(), this->handle, NULL);
	}
	handle = 0;
}

DescriptorPoolVk_::~DescriptorPoolVk_()
{
	if (m_context.isValid())
	{
		destroy();
	}
	else
	{
		Log(Log.Warning, "Attempted to free DescriptorPool after its corresponding context was destroyed.");
	}
}

bool DescriptorSetLayoutVk_::init()
{
	platform::ContextVk& contextVk = native_cast(*getContext());
	VkDescriptorSetLayoutCreateInfo layoutCreateInfo;
	std::vector<VkDescriptorSetLayoutBinding> bindings;
	bindings.resize(getDescriptorDescBindings().size());
	for (pvr::uint32 i = 0; i < getDescriptorDescBindings().size(); ++i)
	{
		DescriptorSetLayoutCreateParam::BindInfo& bindInfo = getDescriptorDescBindings()[i];
		VkDescriptorSetLayoutBinding vkLayoutBinding;
		vkLayoutBinding.descriptorType =  ConvertToVk::descriptorType(bindInfo.descType);
		vkLayoutBinding.descriptorCount = bindInfo.arraySize;
		vkLayoutBinding.pImmutableSamplers = NULL;
		vkLayoutBinding.stageFlags = ConvertToVk::shaderStage(bindInfo.shaderStage);
		vkLayoutBinding.binding = i;
		bindings[i] = vkLayoutBinding;
	}
	layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutCreateInfo.pNext = NULL;
	layoutCreateInfo.flags = 0;
	layoutCreateInfo.bindingCount = (uint32)bindings.size();
	layoutCreateInfo.pBindings = bindings.data();
	return (vk::CreateDescriptorSetLayout(contextVk.getDevice(), &layoutCreateInfo, NULL, &handle) == VK_SUCCESS);
}
}// namespace vulkan
}// namespace api
}// namespace pvr
//!\endcond
