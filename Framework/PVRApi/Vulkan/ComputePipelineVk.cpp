/*!
\brief Contains the OpenGL ES 2/3 implementation of the all-important pvr::api::ComputePipeline object.
\file PVRApi/Vulkan/ComputePipelineVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRApi/Vulkan/ComputePipelineVk.h"
#include "PVRApi/Vulkan/ContextVk.h"
namespace pvr {
namespace api {
namespace vulkan {

ComputePipelineImplVk::~ComputePipelineImplVk() { destroy(); }

ComputePipelineImplVk::ComputePipelineImplVk(GraphicsContext context) : _context(context),
	_pipeCache(VK_NULL_HANDLE) {}

bool ComputePipelineImplVk::init(const ComputePipelineCreateParam& desc)
{
	_createParam = desc;
	_createParam.pipelineLayout =
	  (desc.pipelineLayout.isValid() ? desc.pipelineLayout : PipelineLayout());

	if (!getPipelineLayout().isValid())
	{
		assertion(0, "Invalid PipelineLayout");
		return false;
	}
	vulkan::ComputePipelineCreateInfoVulkan createInfoFactory(desc, _context);
	createInfoFactory.createInfo.flags = 0;
	createInfoFactory.createInfo.flags |= VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT;

	return nativeVk::vkIsSuccessful(vk::CreateComputePipelines(pvr::api::native_cast(*_context).getDevice(),
	                                VK_NULL_HANDLE, 1, &createInfoFactory.createInfo,
	                                NULL, &handle), "Create ComputePipeline");
}

void ComputePipelineImplVk::destroy()
{
	if (_context.isValid() && handle)
	{
		vk::DestroyPipeline(pvr::api::native_cast(*_context).getDevice(), handle, NULL);
		handle = VK_NULL_HANDLE;
	}
	if (_pipeCache != VK_NULL_HANDLE)
	{
		vk::DestroyPipelineCache(pvr::api::native_cast(*_context).getDevice(), _pipeCache, NULL);
		_pipeCache = VK_NULL_HANDLE;
	}
}

void ComputePipelineImplVk::getUniformLocation(const char8**, uint32, int32*) const
{
	assertion(false, "VULKAN DOES NOT SUPPORT SHADER REFLECTION");
}

int32 ComputePipelineImplVk::getUniformLocation(const char8*) const
{
	assertion(false, "VULKAN DOES NOT SUPPORT SHADER REFLECTION");
	return -1;
}


const ComputePipelineCreateParam& ComputePipelineImplVk::getCreateParam() const { return _createParam; }

}

}
}
