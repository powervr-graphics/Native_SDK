/*!*********************************************************************************************************************
\file         PVRApi/OGLES/CommandPoolGles.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        OpenGL ES Implementation details of the CommandPool class.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRApi/OGLES/CommandPoolGles.h"
namespace pvr {
namespace api {
namespace impl {
api::CommandBuffer CommandPool_::allocateCommandBuffer()
{
	CommandPool this_ref = static_cast<gles::CommandPoolGles_*>(this)->getReference();
	api::CommandBuffer commandBuffer;
	native::HCommandBuffer_ dummy;
	commandBuffer.construct(m_context, this_ref, dummy);
	return commandBuffer;
}

api::SecondaryCommandBuffer CommandPool_::allocateSecondaryCommandBuffer()
{
	CommandPool this_ref = static_cast<gles::CommandPoolGles_*>(this)->getReference();
	api::SecondaryCommandBuffer commandBuffer;
	native::HCommandBuffer_ dummy;
	commandBuffer.construct(m_context, this_ref, dummy);
	return commandBuffer;
}
}
}
}
//!\endcond