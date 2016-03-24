/*!*********************************************************************************************************************
\file         PVRApi\OGLES\DescriptorSetGles.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         OpenGL ES implementation of the DescriptorPool class.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRApi/OGLES/DescriptorSetGles.h"
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
