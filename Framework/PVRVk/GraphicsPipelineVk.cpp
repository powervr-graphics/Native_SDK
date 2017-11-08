/*!
\brief Function definitions for the GraphicsPipeline class
\file PVRVk/GraphicsPipelineVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRVk/GraphicsPipelineVk.h"
#include "PVRVk/DeviceVk.h"
#include "PVRVk/PopulateCreateInfoVk.h"
namespace pvrvk {
namespace impl {

bool GraphicsPipeline_::init(VkPipeline vkPipeline, const GraphicsPipelineCreateInfo& desc)
{
	_vkPipeline = vkPipeline;
	_createInfo = desc;
	return true;
}

void GraphicsPipeline_::destroy()
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
			reportDestroyedAfterContext("GraphicsPipeline");
		}
	}
	_parent.reset();
}
}
}// namespace pvrvk
