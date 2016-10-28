/*!*********************************************************************************************************************
\file         PVRApi\OGLES\DescriptorSetGles.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         OpenGL ES implementation of the DescriptorPool class.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRApi/OGLES/DescriptorSetGles.h"
namespace {
using namespace pvr;
template<typename type>
bool validateDescriptorBinding(const types::DescriptorBinding<type>& updateBinding,
                               const types::DescriptorBindingLayout& layoutBinding)
{
	if (!layoutBinding.isValid())
	{
		std::string msg = strings::createFormatted("DescriptorsetUpdate descriptor type does not match with the layout for binding %d",
		                  updateBinding.bindingId);
		assertion(false, msg.c_str());
		Log(msg.c_str());
		return false;
	}
	if (updateBinding.arrayIndex > (int16)layoutBinding.arraySize)
	{
		std::string msg = strings::createFormatted("DescriptorsetUpdate array index is %d but the layout only supports array size %d",
		                  updateBinding.bindingId, layoutBinding.arraySize);
		assertion(false, msg.c_str());
		Log(msg.c_str());
		return false;
	}
	return true;
}
}

namespace pvr {
namespace api {
namespace impl {

native::HDescriptorPool_& DescriptorPool_::getNativeObject()
{
	return static_cast<gles::DescriptorPoolGles_&>(*this);
}

const native::HDescriptorPool_& DescriptorPool_::getNativeObject()const
{
	return static_cast<const gles::DescriptorPoolGles_&>(*this);
}


bool DescriptorSet_::update(const pvr::api::DescriptorSetUpdate& descSet)
{
	return static_cast<gles::DescriptorSetGles_*>(this)->update_(descSet);
}

api::DescriptorSet DescriptorPool_::allocateDescriptorSet(const api::DescriptorSetLayout& layout)
{
	//For OpenGL ES, DescriptorPool is dummy.
	return m_context->createDescriptorSetOnDefaultPool(layout);
}


native::HDescriptorSet_& DescriptorSet_::getNativeObject()
{
	return static_cast<gles::DescriptorSetGles_&>(*this);
}

const native::HDescriptorSet_& DescriptorSet_::getNativeObject()const
{
	return static_cast<const gles::DescriptorSetGles_&>(*this);
}

}

bool gles::DescriptorSetGles_::update_(const DescriptorSetUpdate& descSet)
{
#ifdef DEBUG
	// validate
	const api::DescriptorSetLayoutCreateParam& layoutInfo = m_descSetLayout->getCreateParam();
	const DescriptorSetUpdate::Bindings& updateBindings = descSet.getBindingList();
	for (uint32 i = 0; i < pvr::types::DescriptorBindingDefaults::MaxStorageBuffers; ++i)
	{
		if (updateBindings.storageBuffers[i].isValid())
		{
			auto& binding = layoutInfo.getBinding(updateBindings.storageBuffers[i].bindingId, updateBindings.storageBuffers[i].descType);
			if (!validateDescriptorBinding(updateBindings.storageBuffers[i], binding)) { return false; }
		}
	}
	for (uint32 i = 0; i < pvr::types::DescriptorBindingDefaults::MaxUniformBuffers; ++i)
	{
		if (updateBindings.uniformBuffers[i].isValid())
		{
			auto& binding = layoutInfo.getBinding(updateBindings.uniformBuffers[i].bindingId, updateBindings.uniformBuffers[i].descType);
			if (!validateDescriptorBinding(updateBindings.uniformBuffers[i], binding)) { return false; }
		}
	}
	for (uint32 i = 0; i < pvr::types::DescriptorBindingDefaults::MaxImages; ++i)
	{
		if (updateBindings.images[i].binding.second.isValid() && updateBindings.images[i].isValid())
		{
			auto& binding = layoutInfo.getBinding(updateBindings.images[i].bindingId, updateBindings.images[i].descType);
			if (!validateDescriptorBinding(updateBindings.images[i], binding))
			{
				return false;
			}
		}
	}
#endif
	m_descParam = descSet;
	return true;
}
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
inline RefCountedResource<HDescriptorSetLayout_> createNativeHandle(const RefCountedResource<api::gles::DescriptorSetLayoutGles_>& descSetLayout)
{
	return static_cast<RefCountedResource<api::gles::DescriptorSetLayoutGles_>/**/>(descSetLayout);
}
}
}
//!\endcond
