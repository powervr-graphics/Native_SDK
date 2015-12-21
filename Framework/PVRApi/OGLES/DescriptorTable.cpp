/*!*********************************************************************************************************************
\file         PVRApi\OGLES\DescriptorTable.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         OpenGL ES implementation of the DescriptorPool class.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRApi/ApiObjects/DescriptorTable.h"
#define MAX_DESC_SET_LAYOUT_SUPPORTED_ES 1
namespace pvr {
namespace api {
namespace impl {
pvr::Result::Enum DescriptorPoolImpl::init(const DescriptorPoolCreateParam& createParam,
        DescriptorPoolUsage::Enum usage)
{
	return pvr::Result::Success;
}

void DescriptorPoolImpl::destroy() {}
pvr::Result::Enum DescriptorSetLayoutImpl::init() { return pvr::Result::Success; }

pvr::Result::Enum DescriptorSetImpl::init() { return pvr::Result::Success; }

pvr::Result::Enum PipelineLayoutImpl::init(const PipelineLayoutCreateParam& createParam)
{
	m_desc = createParam;
	return Result::Success;
}

PipelineLayoutImpl::~PipelineLayoutImpl() {}
}// namespace impl


PipelineLayoutCreateParam& PipelineLayoutCreateParam::addDescSetLayout(pvr::uint32 index, const DescriptorSetLayout& descLayout)
{
	if (index > MAX_DESC_SET_LAYOUT_SUPPORTED_ES)
	{
		Log(Logger::Debug, "OpenglES Pipeline only support %i Descriptor SetLayout. Using the First Layout",
		    MAX_DESC_SET_LAYOUT_SUPPORTED_ES);
		index = 0;
	}
	if (index >= this->m_descLayout.size()) { this->m_descLayout.resize(index + 1); }
	this->m_descLayout[index] = descLayout;
	return *this;
}

void DescriptorSetUpdateParam::addImageSampler(pvr::uint16 bindingId, pvr::uint8 arrayIndex,
        const pvr::api::TextureView& texture, const pvr::api::Sampler& sampler)
{
	if (bindingId >= m_combinedSamplerImage.size()) {	m_combinedSamplerImage.resize(bindingId + 1);	}
	m_combinedSamplerImage.push_back(DescriptorBinding<CombinedImageSampler>(bindingId, arrayIndex, std::make_pair(sampler,
	                                 texture)));
}
}
}
//!\endcond
