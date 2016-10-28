/*!*********************************************************************************************************************
\file         PVRApi\OGLES\PipelineLayoutGles.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Definitions for the OpenGL ES 2/3 implementation of the PipelineLayout class
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRApi/OGLES/PipelineLayoutGles.h"
namespace pvr{
namespace api{

const native::HPipelineLayout_& impl::PipelineLayout_::getNativeObject() const
{
	return static_cast<const native::HPipelineLayout_&>(static_cast<const gles::PipelineLayoutGles_&>(*this));
}

native::HPipelineLayout_& impl::PipelineLayout_::getNativeObject()
{
	return static_cast<native::HPipelineLayout_&>(static_cast<gles::PipelineLayoutGles_&>(*this));
}
}
}
//!\endcond
