/*!
\brief Vulkan implementation of the DescriptorSet class.
\file PVRApi/Vulkan/DescriptorSetVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRApi/Vulkan/DescriptorSetVk.h"
#include "PVRApi/Vulkan/TextureVk.h"
#include "PVRApi/Vulkan/SamplerVk.h"
#include "PVRApi/Vulkan/BufferVk.h"
#include "PVRApi/Vulkan/IndirectRayPipelineVk.h"

#ifdef DEBUG
#include <algorithm>
#endif

namespace pvr {
namespace api {

// HELPER FUNCTIONS
namespace {

#ifdef DEBUG
static inline void validateBufferEntries(const pvr::uint32 totalNumberOfBuffers, const VkDescriptorBufferInfo* bufferInfo,
    const pvr::uint32& descSetWritesIndex, const VkWriteDescriptorSet* descSetWritesVk)
{
	debug_assertion(descSetWritesIndex >= totalNumberOfBuffers, "Expected total number of buffers does not match the number of buffers.");
	// validate total number of buffer entries
	pvr::uint32 validBufferInfoEntries = 0;
	for (pvr::uint32 i = 0; i < totalNumberOfBuffers; ++i)
	{
		if (bufferInfo[i].buffer != 0 && bufferInfo[i].range != 0)
		{
			validBufferInfoEntries++;
		}
	}
	debug_assertion(validBufferInfoEntries == totalNumberOfBuffers, "Expected total number of buffers does not match the number of buffers.");
}
#endif
#ifdef DEBUG
static inline void validateLayoutBindings(pvr::api::DescriptorSetLayout& descSetLayout, const pvr::api::DescriptorSetUpdate& descSetUpdate)
{
	const pvr::types::DescriptorBindingLayout* imageLayout = descSetLayout->getCreateParam().getImages();
	const pvr::types::DescriptorBindingLayout* uboLayout = descSetLayout->getCreateParam().getUbos();
	const pvr::types::DescriptorBindingLayout* ssboLayout = descSetLayout->getCreateParam().getSsbos();
	const pvr::types::DescriptorBindingLayout* indirectPipelinesLayout = descSetLayout->getCreateParam().getIndirectRayPipelines();

	struct DescriptorLayout
	{
		pvr::uint16 bindingId;
		types::DescriptorType type;
		pvr::int16 arrayIndex;
	};

	std::vector<DescriptorLayout> layoutTypes;

	pvr::uint16 numDescriptors = 0;
	numDescriptors += descSetLayout->getCreateParam().getTotalArrayElementImageCount();
	numDescriptors += descSetLayout->getCreateParam().getTotalArrayElementUboCount();
	numDescriptors += descSetLayout->getCreateParam().getTotalArrayElementSsboCount();
	numDescriptors += descSetLayout->getCreateParam().getTotalArrayElementIndirectRayPipelineCount();

	layoutTypes.reserve(numDescriptors);

	// add each of the descriptor array elements
	for (pvr::uint16 i = 0; i < descSetLayout->getCreateParam().getImageCount(); i++)
	{
		for (pvr::uint16 j = 0; j < imageLayout[i]._arraySize; j++)
		{
			layoutTypes.push_back(DescriptorLayout{ imageLayout[i]._bindingId, imageLayout[i]._descType, static_cast<pvr::int16>(j) });
		}
	}
	for (pvr::uint16 i = 0; i < descSetLayout->getCreateParam().getUboCount(); i++)
	{
		for (pvr::uint16 j = 0; j < uboLayout[i]._arraySize; j++)
		{
			layoutTypes.push_back(DescriptorLayout{ uboLayout[i]._bindingId, uboLayout[i]._descType, static_cast<pvr::int16>(j) });
		}
	}
	for (pvr::uint16 i = 0; i < descSetLayout->getCreateParam().getSsboCount(); i++)
	{
		for (pvr::uint16 j = 0; j < ssboLayout[i]._arraySize; j++)
		{
			layoutTypes.push_back(DescriptorLayout{ ssboLayout[i]._bindingId, ssboLayout[i]._descType, static_cast<pvr::int16>(j) });
		}
	}
	for (pvr::uint16 i = 0; i < descSetLayout->getCreateParam().getIndirectRayPipelineCount(); i++)
	{
		for (pvr::uint16 j = 0; j < indirectPipelinesLayout[i]._arraySize; j++)
		{
			layoutTypes.push_back(DescriptorLayout{ indirectPipelinesLayout[i]._bindingId, indirectPipelinesLayout[i]._descType, static_cast<pvr::int16>(j) });
		}
	}

	auto* updateImages = descSetUpdate.getImages();
	auto* updateUbos = descSetUpdate.getUbos();
	auto* updateSsbos = descSetUpdate.getSsbos();
	auto* updateIndirectPipelines = descSetUpdate.getIndirectRayPipelines();
	auto* updateAccumulationImages = descSetUpdate.getAccumulationImages();

	std::vector<DescriptorLayout> updateTypes;
	updateTypes.reserve(descSetUpdate.getBindingCount());

	for (pvr::uint16 i = 0; i < descSetUpdate.getImageCount(); i++)
	{
		updateTypes.push_back(DescriptorLayout{ updateImages[i]._bindingId, updateImages[i]._descType, updateImages[i]._arrayIndex });
	}
	for (pvr::uint16 i = 0; i < descSetUpdate.getUboCount(); i++)
	{
		updateTypes.push_back(DescriptorLayout{ updateUbos[i]._bindingId, updateUbos[i]._descType, updateUbos[i]._arrayIndex });
	}
	for (pvr::uint16 i = 0; i < descSetUpdate.getSsboCount(); i++)
	{
		updateTypes.push_back(DescriptorLayout{ updateSsbos[i]._bindingId, updateSsbos[i]._descType, updateSsbos[i]._arrayIndex });
	}
	for (pvr::uint16 i = 0; i < descSetUpdate.getIndirectRayPipelineCount(); i++)
	{
		updateTypes.push_back(DescriptorLayout{ updateIndirectPipelines[i]._bindingId, updateIndirectPipelines[i]._descType, updateIndirectPipelines[i]._arrayIndex });
	}
	for (pvr::uint16 i = 0; i < descSetUpdate.getAccumulationImageCount(); i++)
	{
		updateTypes.push_back(DescriptorLayout{ updateAccumulationImages[i]._bindingId, updateAccumulationImages[i]._descType, updateAccumulationImages[i]._arrayIndex });
	}

	debug_assertion(layoutTypes.size() >= updateTypes.size(), "The number of descriptors updated must be less than or equal to the number of Descriptor set layout items.");

	bool currentUpdateValidated = false;

	for (pvr::uint32 i = 0; i < updateTypes.size(); i++)
	{
		auto currentUpdate = updateTypes[i];
		currentUpdateValidated = false;

		for (pvr::uint32 j = 0; j < layoutTypes.size(); j++)
		{
			if (currentUpdate.bindingId == layoutTypes[j].bindingId && currentUpdate.arrayIndex == layoutTypes[j].arrayIndex)
			{
				debug_assertion(currentUpdate.type == layoutTypes[j].type, "The descriptor set layout type and update types must match.");
				currentUpdateValidated = true;
			}
		}

		debug_assertion(currentUpdateValidated, "Could not validate the current descriptor update type.");
	}
}
#endif
// initialize a VkWriteDescriptorSet
static inline VkWriteDescriptorSet initializeWriteDescSet()
{
	VkWriteDescriptorSet descSet;
	memset(&descSet, 0, sizeof(descSet));
	descSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descSet.pNext = NULL;

	return descSet;
}

// Fill in an ImageInfo entry
static inline void fillImageInfos(const pvr::uint32& index, const DescriptorSetUpdate::Image& imageBinding, VkDescriptorImageInfo* imageInfos)
{
	imageInfos[index].imageView = pvr::api::native_cast(*imageBinding.second).handle;
	if (imageBinding.first._useSampler)
	{
		imageInfos[index].sampler = pvr::api::native_cast(*imageBinding.first._sampler).handle;
	}
	else
	{
		imageInfos[index].sampler = VK_NULL_HANDLE;
	}
	imageInfos[index].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
}

static inline void createImageDescriptorSet(const pvr::uint16 numberOfImageBindings, const pvr::types::DescriptorItemBinding<DescriptorSetUpdate::Image>* imageList,
    const VkDescriptorSet& handle, pvr::types::DescriptorBindingType expectedType, pvr::uint32& descSetWritesIndex, VkDescriptorImageInfo* imageInfos,
    VkWriteDescriptorSet* descSetWritesVk)
{
	if (numberOfImageBindings > 0)
	{
		VkWriteDescriptorSet currentDescSet = initializeWriteDescSet();

		int16 lastBindingId = -1;
		uint16 imageArrayInfoIndex = 0;
		uint16 numArrayElements = 0;

		for (pvr::uint8 i = 0; i < numberOfImageBindings; ++i)
		{
			auto& bindingInfo = imageList[i];
			if (!bindingInfo.isValid()) { continue; }

			// verify we aren't trampling on an existing imageinfo entry
			debug_assertion(imageInfos[imageArrayInfoIndex].imageLayout == 0, "Overwriting existing image infos element.");
			debug_assertion(imageInfos[imageArrayInfoIndex].imageView == 0, "Overwriting existing image infos element.");

			fillImageInfos(imageArrayInfoIndex, bindingInfo._binding, imageInfos);
			imageArrayInfoIndex++;

			if (bindingInfo._bindingId == lastBindingId) // if its a new binding or end of array of the binding
			{
				numArrayElements++;
			}
			else
			{
				numArrayElements = 1;
				currentDescSet.dstSet = handle;
				currentDescSet.dstBinding = bindingInfo._bindingId;
				lastBindingId = bindingInfo._bindingId;

				currentDescSet.dstArrayElement = bindingInfo._arrayIndex;
				currentDescSet.descriptorType = nativeVk::ConvertToVk::descriptorType(bindingInfo._descType);
				currentDescSet.pImageInfo = &imageInfos[imageArrayInfoIndex - 1];

				debug_assertion(expectedType == pvr::types::getDescriptorTypeBinding(bindingInfo._descType), "Descriptor must be an image");

				descSetWritesVk[descSetWritesIndex] = currentDescSet;
				descSetWritesIndex++;
			}

			descSetWritesVk[descSetWritesIndex - 1].descriptorCount = numArrayElements;
		}
	}
}

static inline void createBufferDescriptorSet(const pvr::uint8 numberOfBufferBindings, const pvr::types::DescriptorItemBinding<BufferView>* bufferList,
    const VkDescriptorSet& handle, pvr::types::DescriptorBindingType expectedType, pvr::uint32& descSetWritesIndex,
    VkDescriptorBufferInfo* bufferInfo, VkWriteDescriptorSet* descSetWritesVk, uint16& bufferArrayInfoIndex)
{
	if (numberOfBufferBindings > 0)
	{
		VkWriteDescriptorSet currentDescSet = initializeWriteDescSet();

		int16 lastBindingId = -1;

		uint16 numArrayElements = 0;

		// for each descriptor
		// - Validate the buffer binding id and the array index are linear.
		// - add a new descriptor set if its a new binding or end of array of the binding
		for (pvr::uint8 i = 0; i < numberOfBufferBindings; ++i)
		{
			auto& bindingInfo = bufferList[i];
			if (!bindingInfo.isValid()) { continue; }

			// verify we aren't trampling on an existing bufferinfo entry
			debug_assertion(bufferInfo[bufferArrayInfoIndex].buffer == 0, "Overwriting existing image infos element.");
			debug_assertion(bufferInfo[bufferArrayInfoIndex].range == 0, "Overwriting existing image infos element.");

			auto arrayBinding = bindingInfo._binding;
			bufferInfo[bufferArrayInfoIndex].buffer = pvr::api::native_cast(*arrayBinding->getResource()).buffer;
			bufferInfo[bufferArrayInfoIndex].offset = arrayBinding->getOffset();
			bufferInfo[bufferArrayInfoIndex].range = arrayBinding->getRange();
			bufferArrayInfoIndex++;

			if (bindingInfo._bindingId == lastBindingId) // if its a new binding or end of array of the binding
			{
				numArrayElements++;
			}
			else
			{
				numArrayElements = 1;
				currentDescSet.dstSet = handle;
				currentDescSet.dstBinding = bindingInfo._bindingId;
				lastBindingId = bindingInfo._bindingId;

				currentDescSet.dstArrayElement = bindingInfo._arrayIndex;
				currentDescSet.descriptorType = nativeVk::ConvertToVk::descriptorType(bindingInfo._descType);
				currentDescSet.pBufferInfo = &bufferInfo[bufferArrayInfoIndex - 1];

				debug_assertion(expectedType == pvr::types::getDescriptorTypeBinding(bindingInfo._descType), "Descriptor must be a buffer");

				descSetWritesVk[descSetWritesIndex] = currentDescSet;
				descSetWritesIndex++;
			}

			descSetWritesVk[descSetWritesIndex - 1].descriptorCount = numArrayElements;
		}
	}
}


static inline void addDescriptorBindingLayout(pvr::uint32 arrayIndex, const pvr::types::DescriptorBindingLayout& bindInfo,
    std::vector<VkDescriptorSetLayoutBinding>& bindings)
{
	if (bindInfo.isValid())
	{
		VkDescriptorSetLayoutBinding vkLayoutBinding;
		vkLayoutBinding.descriptorType = nativeVk::ConvertToVk::descriptorType(bindInfo._descType);
		vkLayoutBinding.descriptorCount = bindInfo._arraySize;
		vkLayoutBinding.pImmutableSamplers = NULL;
		vkLayoutBinding.stageFlags = nativeVk::ConvertToVk::shaderStage(bindInfo._shaderStage);
		vkLayoutBinding.binding = bindInfo._bindingId;
		bindings[arrayIndex] = vkLayoutBinding;
	}
}
}

// DESCRIPTOR POOL
namespace vulkan {
api::DescriptorSet DescriptorPoolVk_::allocateDescriptorSet_(const DescriptorSetLayout& layout)
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

bool DescriptorPoolVk_::init(const DescriptorPoolCreateParam& createParam)
{
	VkDescriptorPoolCreateInfo descPoolInfo;
	descPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descPoolInfo.pNext = NULL;
	descPoolInfo.maxSets = createParam.getMaxSetCount();
	descPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	VkDescriptorPoolSize poolSize[pvr::uint32(pvr::types::DescriptorType::Count)];
	pvr::uint32 poolIndex = 0;
	for (pvr::uint32 i = 0; i < pvr::uint32(pvr::types::DescriptorType::Count); ++i)
	{
		pvr::uint32 count = createParam.getDescriptorTypeCount(static_cast<types::DescriptorType>(i));
		if (count)
		{
			poolSize[poolIndex].type = nativeVk::ConvertToVk::descriptorType(static_cast<types::DescriptorType>(i));
			poolSize[poolIndex].descriptorCount = count;
			++poolIndex;
		}
	}// next type
	descPoolInfo.poolSizeCount = poolIndex;
	descPoolInfo.pPoolSizes = poolSize;

	VkDevice dev = native_cast(*getContext()).getDevice();
	VkDescriptorPool pool;
	VkResult res = vk::CreateDescriptorPool(dev, &descPoolInfo, NULL, &pool);
	pvr::api::native_cast(this)->handle = pool;
	return res == VK_SUCCESS;
}
void DescriptorPoolVk_::destroy()
{
	if (handle)
	{
		vk::DestroyDescriptorPool(native_cast(*getContext()).getDevice(), this->handle, NULL);
	}
	handle = 0;
}

DescriptorPoolVk_::~DescriptorPoolVk_()
{
	if (getContext().isValid())
	{
		destroy();
	}
	else
	{
		Log(Log.Warning, "Attempted to free DescriptorPool after its corresponding context was destroyed.");
	}
}
}

// DESCRIPTOR SET
namespace vulkan {
#ifdef DEBUG
static inline bool bindingIdPairComparison(const std::pair<pvr::uint32, uint32>& a, const std::pair<uint32, uint32>& b)
{
	return a.first < b.first;
}
#endif

bool DescriptorSetVk_::update_(const DescriptorSetUpdate& descSet)
{
	_descParam = descSet;
	std::vector<VkDescriptorImageInfo> imageInfos;
	imageInfos.resize(descSet.getImageCount());
	std::vector<VkDescriptorBufferInfo> bufferInfo;
	bufferInfo.resize(descSet.getUboCount() + descSet.getSsboCount());


	std::vector<VkWriteDescriptorSet> descSetWritesVk;
	descSetWritesVk.resize(descSet.getImageCount() + descSet.getUboCount() + descSet.getSsboCount()
	                      );


#ifdef DEBUG
	validateLayoutBindings(_descSetLayout, descSet);
#endif

	pvr::uint32 descSetWritesIndex = 0;
	pvr::uint16 bufferArrayInfoIndex = 0;

	createImageDescriptorSet(descSet.getImageCount(), descSet.getImages(), handle, pvr::types::DescriptorBindingType::Image,
	                         descSetWritesIndex, imageInfos.data(), descSetWritesVk.data());

	createBufferDescriptorSet(descSet.getUboCount(), descSet.getUbos(), handle, pvr::types::DescriptorBindingType::UniformBuffer,
	                          descSetWritesIndex, bufferInfo.data(), descSetWritesVk.data(), bufferArrayInfoIndex);

	createBufferDescriptorSet(descSet.getSsboCount(), descSet.getSsbos(), handle, pvr::types::DescriptorBindingType::StorageBuffer,
	                          descSetWritesIndex, bufferInfo.data(), descSetWritesVk.data(), bufferArrayInfoIndex);


#ifdef DEBUG
	validateBufferEntries(descSet.getUboCount() + descSet.getSsboCount(), bufferInfo.data(), descSetWritesIndex, descSetWritesVk.data());

	// validate that image and buffer entries have been added linearly starting from zero
	std::vector<std::pair<pvr::uint32, pvr::uint32>> bindingIndices(0);

	for (pvr::uint32 i = 0; i < descSet.getImageCount(); i++)
	{
		bindingIndices.push_back(std::make_pair(descSet.getImages()[i]._bindingId, descSet.getImages()[i]._arrayIndex));
	}

	for (pvr::uint32 i = 0; i < descSet.getUboCount(); i++)
	{
		bindingIndices.push_back(std::make_pair(descSet.getUbos()[i]._bindingId, descSet.getUbos()[i]._arrayIndex));
	}

	for (pvr::uint32 i = 0; i < descSet.getSsboCount(); i++)
	{
		bindingIndices.push_back(std::make_pair(descSet.getSsbos()[i]._bindingId, descSet.getSsbos()[i]._arrayIndex));
	}

	std::sort(std::begin(bindingIndices), std::end(bindingIndices), bindingIdPairComparison);

	pvr::uint32 lastBindingIndex = 0;
	pvr::uint32 lastArrayBindingIndex = 0;
	for (pvr::uint32 i = 0; i < descSetWritesIndex; ++i)
	{
		if (i == 0)
		{
			debug_assertion(bindingIndices[i].first == 0, "Binding indices must be linear and must start from zero.");
		}
		else
		{
			debug_assertion(bindingIndices[i].first - 1 == lastBindingIndex || bindingIndices[i].second - 1 == lastArrayBindingIndex,
			                "Binding indices must be linear and must start from zero.");
			lastBindingIndex = bindingIndices[i].first;
			lastArrayBindingIndex = bindingIndices[i].second;
		}
	}
#endif

	if (descSetWritesIndex > 0)
	{
		vk::UpdateDescriptorSets(native_cast(*_descSetLayout->getContext()).getDevice(), descSetWritesIndex, descSetWritesVk.data(), 0, 0);
	}
	return true;
}

void DescriptorSetVk_::destroy()
{
	auto& vkobj = native_cast(*this);
	if (vkobj.handle != VK_NULL_HANDLE)
	{
		if (_descPool->getContext().isValid())
		{
			vk::FreeDescriptorSets(native_cast(*getContext()).getDevice(), native_cast(_descPool)->handle, 1, &native_cast(*this).handle);
		}
		vkobj.handle = VK_NULL_HANDLE;
		_descPool.reset();
		_descSetLayout.reset();
	}
}

void DescriptorSetLayoutVk_::destroy()
{
	if (getContext().isValid())
	{
		if (handle != VK_NULL_HANDLE)
		{
			vk::DestroyDescriptorSetLayout(native_cast(*getContext()).getDevice(), native_cast(*this), NULL);
		}
		getContext().reset();
	}
	clearCreateParam();
}

DescriptorSetVk_::~DescriptorSetVk_()
{
	if (_descPool.isValid())
	{
		if (_descPool->getContext().isValid())
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

DescriptorSetLayoutVk_::~DescriptorSetLayoutVk_()
{
	if (getContext().isValid())
	{
		destroy();
	}
	else
	{
		Log(Log.Warning, "Attempted to free DescriptorSetLayout after its corresponding device was destroyed");
	}
}

bool DescriptorSetLayoutVk_::init()
{
	platform::ContextVk& contextVk = pvr::api::native_cast(*getContext());
	VkDescriptorSetLayoutCreateInfo vkLayoutCreateInfo;
	std::vector<VkDescriptorSetLayoutBinding> bindings;
	bindings.resize(getCreateParam().getBindingCount());

	pvr::uint32 arrayIndex = 0;

	auto imageBindings = getCreateParam().getImages();
	for (pvr::uint8 i = 0; i < getCreateParam().getImageCount(); ++i)
	{
		auto& binding = imageBindings[i];
		addDescriptorBindingLayout(arrayIndex++, binding, bindings);
	}

	auto uboBindings = getCreateParam().getUbos();
	for (pvr::uint8 i = 0; i < getCreateParam().getUboCount(); ++i)
	{
		auto& binding = uboBindings[i];
		addDescriptorBindingLayout(arrayIndex++, binding, bindings);
	}

	auto ssboBindings = getCreateParam().getSsbos();
	for (pvr::uint8 i = 0; i < getCreateParam().getSsboCount(); ++i)
	{
		auto& binding = ssboBindings[i];
		addDescriptorBindingLayout(arrayIndex++, binding, bindings);
	}

	vkLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	vkLayoutCreateInfo.pNext = NULL;
	vkLayoutCreateInfo.flags = 0;
	vkLayoutCreateInfo.bindingCount = (uint32)bindings.size();
	vkLayoutCreateInfo.pBindings = bindings.data();
	return (vk::CreateDescriptorSetLayout(contextVk.getDevice(), &vkLayoutCreateInfo, NULL, &handle) == VK_SUCCESS);
}
}// namespace vulkan
}// namespace api
}// namespace pvr
