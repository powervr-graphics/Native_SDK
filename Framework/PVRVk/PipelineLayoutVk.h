/*!
\brief The Pipeline Layout class
\file PVRVk/PipelineLayoutVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVRVk/DeviceVk.h"
#include <array>

namespace pvrvk {
typedef std::array<DescriptorSetLayout, static_cast<uint32_t>(FrameworkCaps::MaxDescriptorSetBindings)> DescriptorSetLayoutList;//!< Descriptor set layout Array type
/// <summary>Pipeline Layout create information. The descriptor set layouts must be known to create a Pipeline layout.
/// </summary>
struct PipelineLayoutCreateInfo
{
	/// <summary>Add a descriptor set layout to this pipeline layout. Added to the end of the list of layouts.
	/// </summary>
	/// <param name="descLayout">A descriptor set layout</param>
	/// <returns>this (allow chaining)</returns>
	PipelineLayoutCreateInfo& addDescSetLayout(const DescriptorSetLayout& descLayout)
	{
		assertion(size < static_cast<uint32_t>(FrameworkCaps::MaxDescriptorSetBindings),
		          "PipelineLayoutCreateInfo: Descriptor Set index cannot be 4 or greater");
		_descLayout[size++] = descLayout;
		return *this;
	}

	/// <summary>Add a descriptor set layout to this pipeline layout. Added to the specified index.</summary>
	/// <param name="index">The index where the layout will be created on</param>
	/// <param name="descLayout">A descriptor set layout</param>
	/// <returns>this (allow chaining)</returns>
	PipelineLayoutCreateInfo& setDescSetLayout(uint32_t index, const DescriptorSetLayout& descLayout)
	{
#ifdef DEBUG
		if (index >= static_cast<uint32_t>(FrameworkCaps::MaxDescriptorSetBindings))
		{
			assertion(index < static_cast<uint32_t>(FrameworkCaps::MaxDescriptorSetBindings), "PipelineLayoutCreateInfo: Descriptor Set index cannot be 4 or greater");
			Log("PipelineLayoutCreateInfo::getDescriptorSetLayout - Invalid DescriptorSetLayout Index");
		}
#endif
		if (index >= static_cast<uint32_t>(size)) { size = static_cast<uint8_t>(index) + 1; }
		_descLayout[index] = descLayout;
		return *this;
	}

	/// <summary>Return number of descriptor set layouts</summary>
	/// <returns>Return number of descriptor set layouts</returns>
	uint32_t getNumDescriptorSetLayouts()const { return size; }

	/// <summary>Get descriptor set layout</summary>
	/// <param name="index">Descriptor set layout index</param>
	/// <returns>Descritpor set layout</returns>
	const DescriptorSetLayout& getDescriptorSetLayout(uint32_t index)const
	{
#ifdef DEBUG
		if (index >= size)
		{
			assertion(false, "Invalid DescriptorSetLayout Index");
			Log("PipelineLayoutCreateInfo::getDescriptorSetLayout - Invalid DescriptorSetLayout Index");
		}
#endif
		return _descLayout[index];
	}

	/// <summary>Clear the entries</summary>
	void clear()
	{
		for (size_t i = 0; i < size; ++i)
		{
			_descLayout[i].reset();
		}
		size = 0;
	}

	/// <summary>Get all descriptor set layouts</summary>
	/// <returns>An array of 4 descriptor set layouts. Unused one will be empty references (isNull() returns true)
	/// </returns>
	const DescriptorSetLayoutList& getDescriptorSetLayouts()const
	{
		return _descLayout;
	}

	/// <summary>Return if this object is equal</summary>
	/// <param name="rhs">Object to compare with</param>
	/// <returns>Return true if equal</returns>
	bool operator==(const PipelineLayoutCreateInfo& rhs) const
	{
		if (size != rhs.size) { return false; }
		for (size_t i = 0; i < size; ++i)
		{
			if (_descLayout[i] != rhs._descLayout[i]) { return false; }
		}
		return true;
	}

	/// <summary>Set push constant range</summary>
	/// <param name="index">Push constant range index</param>
	/// <param name="pushConstantRange">push constant range to set</param>
	void setPushConstantRange(uint32_t index, const PushConstantRange& pushConstantRange)
	{
		if (pushConstantRange.size == 0)
		{
			debug_assertion(false, "Push constant range size must not be be 0");
			Log("Push constant range size must not be be 0");
		}

		if (index >= _pushConstantRange.size()) { _pushConstantRange.resize((index + 1) - _pushConstantRange.size()); }
		_pushConstantRange[index] = pushConstantRange;
	}

	/// <summary>Get push constant range</summary>
	/// <param name="index">Push cinstant range index</param>
	/// <returns>Returns push constant range</returns>
	const PushConstantRange& getPushConstantRange(uint32_t index)const
	{
		return _pushConstantRange.at(index);
	}

	/// <summary>Get number of push constants</summary>
	/// <returns>Number of push constants</returns>
	uint32_t getNumPushConstantRanges()const { return static_cast<uint32_t>(_pushConstantRange.size()); }

	/// <summary>PipelineLayoutCreateInfo</summary>
	PipelineLayoutCreateInfo() : size(0) {}
private:
	bool isValidPushConstantRange(uint32_t index) { return _pushConstantRange[index].size != 0; }
	friend class ::pvrvk::impl::PipelineLayout_;
	friend class ::pvrvk::impl::GraphicsPipeline_;
	std::array<DescriptorSetLayout, FrameworkCaps::MaxDescriptorSetBindings> _descLayout;
	uint8_t size;
	std::vector<PushConstantRange> _pushConstantRange;
};

namespace impl {
/// <summary>Vulkan implementation of the PipelineLayout class.</summary>
class PipelineLayout_
{
public:
	DECLARE_NO_COPY_SEMANTICS(PipelineLayout_)

	/// <summary>Get a descriptor set layout used by this pipeline layout</summary>
	/// <param name="index">Layout index</param>
	/// <returns>std::vector<pvrvk::api::DescriptorSetLayout>&</returns>
	const DescriptorSetLayout& getDescriptorSetLayout(uint32_t index)const
	{
		assertion(index < _createInfo.size && "Invalid Index");
		return _createInfo._descLayout[index];
	}

	/// <summary>Get all the descriptor set layouts used by this object as (raw datastructure)</summary>
	/// <returns>The underlying container of all descriptor set layouts used.</returns>
	const DescriptorSetLayoutSet& getDescriptorSetLayouts()const
	{
		return _createInfo._descLayout;
	}

	/// <summary>Get number of descriptorSet layout</summary>
	/// <returns>the number of descriptor set layouts</returns>
	uint32_t getNumDescriptorSetLayouts()const { return _createInfo.size; }

	/// <summary>Return create param</summary>
	/// <returns>const PipelineLayoutCreateInfo&</returns>
	const PipelineLayoutCreateInfo& getCreateInfo()const { return _createInfo; }

	/// <summary>Get vulkan object (const)</summary>
	/// <returns>const VkPipelineLayout&</returns>
	const VkPipelineLayout& getNativeObject()const { return _vkPipeLayout; }
private:
	template<typename> friend struct ::pvrvk::RefCountEntryIntrusive;
	friend class ::pvrvk::impl::Device_;

	bool init(const PipelineLayoutCreateInfo& createInfo);

	/// <summary>dtor</summary>
	~PipelineLayout_() { destroy(); }

	/// <summary>Release all resources held by this object</summary>
	void destroy()
	{
		if (_vkPipeLayout != VK_NULL_HANDLE)
		{
			if (_device.isValid())
			{
				vk::DestroyPipelineLayout(_device->getNativeObject(), _vkPipeLayout, nullptr);
				_vkPipeLayout = VK_NULL_HANDLE;
				_device.reset();
			}
			else
			{
				reportDestroyedAfterContext("PipelineLayout");
			}
		}
	}

	/// <summary>INTERNAL. Use context->createPipelineLayout to create this object</summary>
	PipelineLayout_(const DeviceWeakPtr& device) : _device(device), _vkPipeLayout(VK_NULL_HANDLE) {}

	DeviceWeakPtr _device;
	PipelineLayoutCreateInfo _createInfo;
	VkPipelineLayout _vkPipeLayout;

};
}// namespace impl
}// namespace pvrvk
