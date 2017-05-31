/*!
\brief Definition of the Vulkan implementation of the all important GraphicsPipeline class
\file PVRApi/Vulkan/GraphicsPipelineVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRApi/Vulkan/GraphicsPipelineVk.h"
#include "PVRApi/Vulkan/ContextVk.h"
namespace pvr {
namespace api {
namespace vulkan {
GraphicsPipelineImplVk::~GraphicsPipelineImplVk() { destroy(); }

GraphicsPipelineImplVk::GraphicsPipelineImplVk(GraphicsContext context) : _context(context),
	_pipeCache(VK_NULL_HANDLE), _parent(NULL) {}

bool GraphicsPipelineImplVk::init(const GraphicsPipelineCreateParam& desc, const ParentableGraphicsPipeline& parent)
{
	_createParam = desc;
	_createParam.pipelineLayout =
	  (desc.pipelineLayout.isValid() ? desc.pipelineLayout :
	   (parent.isValid() ? parent->getPipelineLayout() : PipelineLayout()));

	if (!getPipelineLayout().isValid())
	{
		assertion(0, "Invalid PipelineLayout");
		return false;
	}
	vulkan::GraphicsPipelineCreateInfoVulkan createInfoFactory(desc, _context, parent);
	createInfoFactory.createInfo.flags = (parent.isValid() ? VK_PIPELINE_CREATE_DERIVATIVE_BIT : 0);
	createInfoFactory.createInfo.flags |= VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT;

	return nativeVk::vkIsSuccessful(vk::CreateGraphicsPipelines(pvr::api::native_cast(*_context).getDevice(),
	                                VK_NULL_HANDLE, 1, &createInfoFactory.createInfo,
	                                NULL, &handle), "Create GraphicsPipeline");
}

const VertexInputBindingInfo* GraphicsPipelineImplVk::getInputBindingInfo(uint16) const
{
	return NULL;
}

const VertexAttributeInfoWithBinding* GraphicsPipelineImplVk::getAttributesInfo(uint16) const
{
	return NULL;
}

void GraphicsPipelineImplVk::destroy()
{
	if (_context.isValid() && handle)
	{
		vk::DestroyPipeline(pvr::api::native_cast(*_context).getDevice(), handle, NULL);
		handle = VK_NULL_HANDLE;
	}
	if (_pipeCache.handle != VK_NULL_HANDLE)
	{
		vk::DestroyPipelineCache(pvr::api::native_cast(*_context).getDevice(), _pipeCache, NULL);
		_pipeCache = VK_NULL_HANDLE;
	}
	_parent = 0;
}

void GraphicsPipelineImplVk::getUniformLocation(const char8**, uint32, int32*) const
{
	assertion(false, "VULKAN DOES NOT SUPPORT SHADER REFLECTION");
}

int32 GraphicsPipelineImplVk::getUniformLocation(const char8*) const
{
	assertion(false, "VULKAN DOES NOT SUPPORT SHADER REFLECTION");
	return -1;
}

int32 GraphicsPipelineImplVk::getAttributeLocation(const char8*) const
{
	assertion(false, "VULKAN DOES NOT SUPPORT SHADER REFLECTION");
	return -1;
}

void GraphicsPipelineImplVk::getAttributeLocation(const char8**, uint32, int32*) const
{
	assertion(false, "VULKAN DOES NOT SUPPORT SHADER REFLECTION");
}

uint8 GraphicsPipelineImplVk::getNumAttributes(uint16) const
{
	assertion(false, "VULKAN DOES NOT SUPPORT SHADER REFLECTION");
	return 0;
}

const PipelineLayout& GraphicsPipelineImplVk::getPipelineLayout() const { return _createParam.pipelineLayout; }

const GraphicsPipelineCreateParam& GraphicsPipelineImplVk::getCreateParam() const { return _createParam; }

ParentableGraphicsPipelineImplVk::ParentableGraphicsPipelineImplVk(GraphicsContext context) : GraphicsPipelineImplVk(context) {}

bool ParentableGraphicsPipelineImplVk::init(const GraphicsPipelineCreateParam& desc, const api::ParentableGraphicsPipeline& parent)
{
	_createParam = desc;
	if (!desc.pipelineLayout.isValid())
	{
		assertion(false, "Invalid PipelineLayout");
		return false;
	}
	VkPipelineCacheCreateInfo cacheCreateInfo{};
	cacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	if (vk::CreatePipelineCache(pvr::api::native_cast(_context)->getDevice(), &cacheCreateInfo,
	                            NULL, &_pipeCache.handle) != VK_SUCCESS)
	{
		Log("Failed to create Pipeline Cache");
		return false;
	}
	vulkan::GraphicsPipelineCreateInfoVulkan createInfoFactory(desc, _context, parent);
	createInfoFactory.createInfo.flags = VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT | (parent.isValid() ? VK_PIPELINE_CREATE_DERIVATIVE_BIT : 0);

	return nativeVk::vkIsSuccessful(vk::CreateGraphicsPipelines(pvr::api::native_cast(_context)->getDevice(),
	                                _pipeCache, 1, &createInfoFactory.createInfo, NULL, &handle), "Create Parentable GraphicsPipeline");
}



}
}
}
