/*!*********************************************************************************************************************
\file         PVRApi/Vulkan/GraphicsPipelineVk.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief		  Definition of the Vulkan implementation of the all important GraphicsPipeline class
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRApi/Vulkan/GraphicsPipelineVk.h"
#include "PVRApi/Vulkan/ContextVk.h"
namespace pvr {
namespace api {
namespace vulkan {

GraphicsPipelineImplVk::~GraphicsPipelineImplVk() { destroy(); }

GraphicsPipelineImplVk::GraphicsPipelineImplVk(GraphicsContext context) : m_context(context),
	m_pipeCache(VK_NULL_HANDLE), m_parent(NULL) {}

bool GraphicsPipelineImplVk::init(const GraphicsPipelineCreateParam& desc, impl::ParentableGraphicsPipeline_ *parent)
{
	m_pipeInfo = desc;
	m_pipeInfo.pipelineLayout =
	  (desc.pipelineLayout.isValid() ? desc.pipelineLayout :
	   (parent ? parent->getPipelineLayout() : PipelineLayout()));

	if (!getPipelineLayout().isValid())
	{
		assertion(0, "Invalid PipelineLayout");
		return false;
	}
	vulkan::GraphicsPipelineCreateInfoVulkan createInfoFactory(desc, m_context, parent);
	createInfoFactory.createInfo.flags = (parent ? VK_PIPELINE_CREATE_DERIVATIVE_BIT : 0);
	createInfoFactory.createInfo.flags |= VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT;

	return pvr::vkIsSuccessful(vk::CreateGraphicsPipelines(pvr::api::native_cast(*m_context).getDevice(),
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
	if (m_context.isValid() && handle)
	{
		vk::DestroyPipeline(pvr::api::native_cast(*m_context).getDevice(), handle, NULL);
		handle = VK_NULL_HANDLE;
	}
	if (m_pipeCache != VK_NULL_HANDLE)
	{
		vk::DestroyPipelineCache(pvr::api::native_cast(*m_context).getDevice(), m_pipeCache, NULL);
		m_pipeCache = VK_NULL_HANDLE;
	}
	m_parent = 0;
}

void GraphicsPipelineImplVk::getUniformLocation(const char8**, uint32 , int32*) const
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

void GraphicsPipelineImplVk::getAttributeLocation(const char8**, uint32 , int32*) const
{
	assertion(false, "VULKAN DOES NOT SUPPORT SHADER REFLECTION");
}

uint8 GraphicsPipelineImplVk::getNumAttributes(uint16) const
{
	assertion(false, "VULKAN DOES NOT SUPPORT SHADER REFLECTION");
	return 0;
}

const PipelineLayout& GraphicsPipelineImplVk::getPipelineLayout() const { return m_pipeInfo.pipelineLayout; }

const native::HPipeline_& GraphicsPipelineImplVk::getNativeObject() const { return *this; }

native::HPipeline_& GraphicsPipelineImplVk::getNativeObject() { return *this; }

void GraphicsPipelineImplVk::bind() {}

const GraphicsPipelineCreateParam& GraphicsPipelineImplVk::getCreateParam() const { return m_pipeInfo; }

ParentableGraphicsPipelineImplVk::ParentableGraphicsPipelineImplVk(GraphicsContext context) : GraphicsPipelineImplVk(context) {}

bool ParentableGraphicsPipelineImplVk::init(const GraphicsPipelineCreateParam& desc)
{
	m_pipeInfo = desc;
	if (!desc.pipelineLayout.isValid())
	{
		assertion(false, "Invalid PipelineLayout");
		return false;
	}
	VkPipelineCacheCreateInfo cacheCreateInfo{};
	cacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	if (vk::CreatePipelineCache(pvr::api::native_cast(m_context)->getDevice(), &cacheCreateInfo,
	                            NULL, &m_pipeCache.handle) != VK_SUCCESS)
	{
		Log("Failed to create Pipeline Cache");
		return false;
	}
	vulkan::GraphicsPipelineCreateInfoVulkan createInfoFactory(desc, m_context, NULL);
	createInfoFactory.createInfo.flags = VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT;

	return pvr::vkIsSuccessful(vk::CreateGraphicsPipelines(pvr::api::native_cast(m_context)->getDevice(),
	                           m_pipeCache, 1, &createInfoFactory.createInfo, NULL, &handle), "Create Parentable GraphicsPipeline");
}



}
}
}
//!\endcond