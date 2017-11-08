/*!
\brief The PVRVk Compute Pipeline class, an interface to a VkPipeline that has been
created for the VK_PIPELINE_BINDING_POINT_COMPUTE
\file PVRVk/ComputePipelineVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRVk/DeviceVk.h"
#include "PVRVk/PipelineConfigVk.h"
namespace pvrvk {
/// <summary>Compute pipeline create parameters.</summary>
struct ComputePipelineCreateInfo
{
public:
	VkPipelineCreateFlags flags;//!< Pipeline create flags
	ShaderStageCreateInfo computeShader;  //!< Compute shader information
	PipelineLayout pipelineLayout; //!< Compute pipeline information
	ComputePipeline basePipeline;//!< Pipelineline handle to derive from
	uint32_t basePipelineIndex;//!< Is an index to the ComputePipelineCreateInfo parameter to use as a pipeline to derive from
	ComputePipelineCreateInfo() : basePipelineIndex(static_cast<uint32_t>(-1)), flags(VkPipelineCreateFlags(0)) {}
};
namespace impl {

/// <summary> Vulkan Computepipeline wrapper<summary>
class ComputePipeline_
{
public:
	DECLARE_NO_COPY_SEMANTICS(ComputePipeline_)

	/// <summary>Return pipeline layout.</summary>
	/// <returns>const PipelineLayout&</returns>
	const PipelineLayout& getPipelineLayout()const
	{
		return _createInfo.pipelineLayout;
	}

	/// <summary>return pipeline create param used to create the child pipeline</summary>
	/// <returns>const ComputePipelineCreateInfo&</returns>
	const ComputePipelineCreateInfo& getCreateInfo()const { return _createInfo; }

	/// <summary>return pipeline create param used to create the child pipeline(const)</summary>
	/// <returns>const ComputePipelineCreateInfo&</returns>
	const VkPipeline& getNativeObject()const { return _vkPipeline; }

private:
	template<typename> friend struct ::pvrvk::RefCountEntryIntrusive;
	friend class ::pvrvk::impl::Device_;
	~ComputePipeline_() { destroy(); }
	void destroy()
	{
		if (_vkPipeline != VK_NULL_HANDLE || _pipeCache != VK_NULL_HANDLE)
		{
			if (_device.isValid())
			{
				if (_vkPipeline != VK_NULL_HANDLE)
				{
					vk::DestroyPipeline(_device->getNativeObject(), _vkPipeline, NULL);
					_vkPipeline = VK_NULL_HANDLE;
				}
				if (_pipeCache != VK_NULL_HANDLE)
				{
					vk::DestroyPipelineCache(_device->getNativeObject(), _pipeCache, NULL);
					_pipeCache = VK_NULL_HANDLE;
				}
				_device.reset();
			}
			else
			{
				reportDestroyedAfterContext("ComputePipeline");
			}
		}
	}

	ComputePipeline_(DeviceWeakPtr device, const ComputePipelineCreateInfo& createInfo,
	                 VkPipeline vkPipeline) : _device(device), _createInfo(createInfo),
		_pipeCache(VK_NULL_HANDLE), _vkPipeline(vkPipeline) {}

	pvrvk::ComputePipelineCreateInfo _createInfo;
	DeviceWeakPtr _device;
	VkPipelineCache _pipeCache;
	VkPipeline _vkPipeline;
};
}
}// namespace pvrvk
