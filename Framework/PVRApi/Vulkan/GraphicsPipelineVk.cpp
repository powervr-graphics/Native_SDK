<<<<<<< HEAD
/*!*********************************************************************************************************************
\file         PVRApi/Vulkan/GraphicsPipelineVk.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief		  Definition of the Vulkan implementation of the all important GraphicsPipeline class
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
=======
/*!
\brief Definition of the Vulkan implementation of the all important GraphicsPipeline class
\file PVRApi/Vulkan/GraphicsPipelineVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
>>>>>>> 1776432f... 4.3
#include "PVRApi/Vulkan/GraphicsPipelineVk.h"
#include "PVRApi/Vulkan/ContextVk.h"
namespace pvr {
namespace api {
namespace vulkan {
<<<<<<< HEAD

GraphicsPipelineImplVk::~GraphicsPipelineImplVk() { destroy(); }

GraphicsPipelineImplVk::GraphicsPipelineImplVk(GraphicsContext context) : m_context(context),
	m_pipeCache(VK_NULL_HANDLE), m_parent(NULL) {}

bool GraphicsPipelineImplVk::init(const GraphicsPipelineCreateParam& desc, impl::ParentableGraphicsPipeline_ *parent)
{
	m_pipeInfo = desc;
	m_pipeInfo.pipelineLayout =
	  (desc.pipelineLayout.isValid() ? desc.pipelineLayout :
	   (parent ? parent->getPipelineLayout() : PipelineLayout()));
=======
GraphicsPipelineImplVk::~GraphicsPipelineImplVk() { destroy(); }

GraphicsPipelineImplVk::GraphicsPipelineImplVk(GraphicsContext context) : _context(context),
	_pipeCache(VK_NULL_HANDLE), _parent(NULL) {}

bool GraphicsPipelineImplVk::init(const GraphicsPipelineCreateParam& desc, const ParentableGraphicsPipeline& parent)
{
	_createParam = desc;
	_createParam.pipelineLayout =
	  (desc.pipelineLayout.isValid() ? desc.pipelineLayout :
	   (parent.isValid() ? parent->getPipelineLayout() : PipelineLayout()));
>>>>>>> 1776432f... 4.3

	if (!getPipelineLayout().isValid())
	{
		assertion(0, "Invalid PipelineLayout");
		return false;
	}
<<<<<<< HEAD
	vulkan::GraphicsPipelineCreateInfoVulkan createInfoFactory(desc, m_context, parent);
	createInfoFactory.createInfo.flags = (parent ? VK_PIPELINE_CREATE_DERIVATIVE_BIT : 0);
	createInfoFactory.createInfo.flags |= VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT;

	return pvr::vkIsSuccessful(vk::CreateGraphicsPipelines(pvr::api::native_cast(*m_context).getDevice(),
	                           VK_NULL_HANDLE, 1, &createInfoFactory.createInfo,
	                           NULL, &handle), "Create GraphicsPipeline");
=======
	vulkan::GraphicsPipelineCreateInfoVulkan createInfoFactory(desc, _context, parent);
	createInfoFactory.createInfo.flags = (parent.isValid() ? VK_PIPELINE_CREATE_DERIVATIVE_BIT : 0);
	createInfoFactory.createInfo.flags |= VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT;

	return nativeVk::vkIsSuccessful(vk::CreateGraphicsPipelines(pvr::api::native_cast(*_context).getDevice(),
	                                VK_NULL_HANDLE, 1, &createInfoFactory.createInfo,
	                                NULL, &handle), "Create GraphicsPipeline");
>>>>>>> 1776432f... 4.3
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
<<<<<<< HEAD
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
=======
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
>>>>>>> 1776432f... 4.3
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

<<<<<<< HEAD
void GraphicsPipelineImplVk::getAttributeLocation(const char8**, uint32 , int32*) const
=======
void GraphicsPipelineImplVk::getAttributeLocation(const char8**, uint32, int32*) const
>>>>>>> 1776432f... 4.3
{
	assertion(false, "VULKAN DOES NOT SUPPORT SHADER REFLECTION");
}

uint8 GraphicsPipelineImplVk::getNumAttributes(uint16) const
{
	assertion(false, "VULKAN DOES NOT SUPPORT SHADER REFLECTION");
	return 0;
}

<<<<<<< HEAD
const PipelineLayout& GraphicsPipelineImplVk::getPipelineLayout() const { return m_pipeInfo.pipelineLayout; }

const native::HPipeline_& GraphicsPipelineImplVk::getNativeObject() const { return *this; }

native::HPipeline_& GraphicsPipelineImplVk::getNativeObject() { return *this; }

void GraphicsPipelineImplVk::bind() {}

const GraphicsPipelineCreateParam& GraphicsPipelineImplVk::getCreateParam() const { return m_pipeInfo; }

ParentableGraphicsPipelineImplVk::ParentableGraphicsPipelineImplVk(GraphicsContext context) : GraphicsPipelineImplVk(context) {}

bool ParentableGraphicsPipelineImplVk::init(const GraphicsPipelineCreateParam& desc)
{
	m_pipeInfo = desc;
=======
const PipelineLayout& GraphicsPipelineImplVk::getPipelineLayout() const { return _createParam.pipelineLayout; }

const GraphicsPipelineCreateParam& GraphicsPipelineImplVk::getCreateParam() const { return _createParam; }

ParentableGraphicsPipelineImplVk::ParentableGraphicsPipelineImplVk(GraphicsContext context) : GraphicsPipelineImplVk(context) {}

bool ParentableGraphicsPipelineImplVk::init(const GraphicsPipelineCreateParam& desc, const api::ParentableGraphicsPipeline& parent)
{
	_createParam = desc;
>>>>>>> 1776432f... 4.3
	if (!desc.pipelineLayout.isValid())
	{
		assertion(false, "Invalid PipelineLayout");
		return false;
	}
	VkPipelineCacheCreateInfo cacheCreateInfo{};
	cacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
<<<<<<< HEAD
	if (vk::CreatePipelineCache(pvr::api::native_cast(m_context)->getDevice(), &cacheCreateInfo,
	                            NULL, &m_pipeCache.handle) != VK_SUCCESS)
=======
	if (vk::CreatePipelineCache(pvr::api::native_cast(_context)->getDevice(), &cacheCreateInfo,
	                            NULL, &_pipeCache.handle) != VK_SUCCESS)
>>>>>>> 1776432f... 4.3
	{
		Log("Failed to create Pipeline Cache");
		return false;
	}
<<<<<<< HEAD
	vulkan::GraphicsPipelineCreateInfoVulkan createInfoFactory(desc, m_context, NULL);
	createInfoFactory.createInfo.flags = VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT;

	return pvr::vkIsSuccessful(vk::CreateGraphicsPipelines(pvr::api::native_cast(m_context)->getDevice(),
	                           m_pipeCache, 1, &createInfoFactory.createInfo, NULL, &handle), "Create Parentable GraphicsPipeline");
=======
	vulkan::GraphicsPipelineCreateInfoVulkan createInfoFactory(desc, _context, parent);
	createInfoFactory.createInfo.flags = VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT | (parent.isValid() ? VK_PIPELINE_CREATE_DERIVATIVE_BIT : 0);

	return nativeVk::vkIsSuccessful(vk::CreateGraphicsPipelines(pvr::api::native_cast(_context)->getDevice(),
	                                _pipeCache, 1, &createInfoFactory.createInfo, NULL, &handle), "Create Parentable GraphicsPipeline");
>>>>>>> 1776432f... 4.3
}



}
}
}
<<<<<<< HEAD
//!\endcond
=======
>>>>>>> 1776432f... 4.3
