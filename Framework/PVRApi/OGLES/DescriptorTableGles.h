/*!*********************************************************************************************************************
\file         PVRApi\OGLES\DescriptorTableGles.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief		Definition of the OpenGL ES implementation of the DescriptorTable and supporting classes
***********************************************************************************************************************/

#pragma once
#include "PVRApi/ApiObjects/DescriptorTable.h"
#include "PVRApi/OGLES/NativeObjectsGles.h"
#include "PVRApi/OGLES/ContextGles.h"
namespace pvr {
namespace api {
namespace impl {

/*!*********************************************************************************************************************
\brief ctor. OpenGL ES implementation of a DescriptorSet.
***********************************************************************************************************************/
class DescriptorSetLayoutGlesImpl : public native::HDescriptorSetLayout_ , public DescriptorSetLayoutImpl
{
public:
	/*!*********************************************************************************************************************
	\brief ctor. Do not use directly, use context->createDescriptorSet.
	***********************************************************************************************************************/
	DescriptorSetLayoutGlesImpl(GraphicsContext& context, const DescriptorSetLayoutCreateParam& desc) :
		DescriptorSetLayoutImpl(context, desc) {}

	pvr::Result::Enum init() { return pvr::Result::Success; }

	/*!*********************************************************************************************************************
	\brief dtor.
	***********************************************************************************************************************/
	~DescriptorSetLayoutGlesImpl() {}
};

/*!*********************************************************************************************************************
\brief OpenGL ES implementation of a DescriptorSet.
***********************************************************************************************************************/
class DescriptorSetGlesImpl : public DescriptorSetImpl, native::HDescriptorSet_
{
public:
	/*!*********************************************************************************************************************
	\brief ctor. 
	***********************************************************************************************************************/
	DescriptorSetGlesImpl(const DescriptorSetLayout& descSetLayout, const DescriptorPool& pool) :
		DescriptorSetImpl(descSetLayout, pool) {}

	pvr::Result::Enum init() { return pvr::Result::Success; }

	pvr::Result::Enum update(const pvr::api::DescriptorSetUpdateParam& descSet)
	{
		m_descParam = descSet;
		return pvr::Result::Success;
	}
	void bind(IGraphicsContext& device, pvr::uint32 dynamicOffset)const
	{
		platform::ContextGles& contextES =  static_cast<platform::ContextGles&>(device);
		// bind the ubos
		for (pvr::uint16 j = 0; j < m_descParam.m_ubos.size(); ++j)
		{
			auto const& walk = m_descParam.m_ubos[j];
			if (!walk.binding.isNull()) { walk.binding->bind(device, walk.bindingId, dynamicOffset); }

		}
		// bind the combined texture and samplers
		for (pvr::uint16 j = 0; j < m_descParam.m_combinedSamplerImage.size(); ++j)
		{
			auto const& walk = m_descParam.m_combinedSamplerImage[j];

			if (!walk.binding.second.isNull())
			{
				walk.binding.second->bind(device, walk.bindingId);//bind the texture
				if (walk.binding.first.isNull())
				{
					contextES.getDefaultSampler()->bind(device, walk.bindingId);
				}
			}
			if (!walk.binding.first.isNull())
			{
				walk.binding.first->bind(device, walk.bindingId);// bind the sampler
			}
		}

		for (pvr::uint16 j = 0; j < m_descParam.m_ssbos.size(); ++j)
		{
			auto const& walk = m_descParam.m_ssbos[j];
			if (!walk.binding.isNull())	{ walk.binding->bind(device, walk.bindingId, dynamicOffset); }

		}
		debugLogApiError("DescriptorSet::bind exit");
	}

	/*!*********************************************************************************************************************
	\brief dtor.
	***********************************************************************************************************************/
	~DescriptorSetGlesImpl() {}
};

}

typedef RefCountedResource<impl::DescriptorSetGlesImpl>DescriptorSetGles;

typedef RefCountedResource<impl::DescriptorSetLayoutGlesImpl>DescriptorSetLayoutGles;
}

namespace native {
/*!*********************************************************************************************************************
\brief Get the OpenGL ES DescSetLayout object underlying a PVRApi DescSetLayout object.
\return A smart pointer wrapper containing the OpenGL ES DescSetLayout.
\description The smart pointer returned by this function works normally with the reference counting, and shares it with the
rest of the references to this object, keeping the underlying OpenGL ES object alive even if all other
references to it (including the one that was passed to this function) are released. Release when done using it to
avoid leaking the object.
***********************************************************************************************************************/
//inline RefCountedResource<HDescriptorSetLayout_> getNativeHandle(const RefCountedResource<api::impl::DescriptorSetLayoutGlesImpl>&
//        descSetLayout)
//{
//	return static_cast<RefCountedResource<native::HDescriptorSetLayout_>/**/>
//	       (static_cast<RefCountedResource<api::impl::DescriptorSetLayoutGlesImpl>/**/>(descSetLayout));
//}

/*!*********************************************************************************************************************
\brief Get the OpenGL ES DescSetLayout object underlying a PVRApi DescSetLayout object.
\return A OpenGL ES DescSetLayout. For immediate use only.
\description The object returned by this function will only be kept alive as long as there are other references to it. If all
other references to it are released or out of scope, the Shader returned by this function will be deleted and invalid.
***********************************************************************************************************************/
//inline HDescriptorSetLayout_::NativeType useNativeHandle(const RefCountedResource<api::impl::DescriptorSetLayoutGlesImpl>&
//        descSetLayout)
//{
//	return static_cast<const api::impl::DescriptorSetLayoutGlesImpl&>(*descSetLayout).handle;
//}
}
}