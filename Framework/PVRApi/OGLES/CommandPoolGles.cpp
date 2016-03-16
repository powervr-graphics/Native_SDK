#include "PVRApi/OGLES/CommandPoolGles.h"
namespace pvr {
namespace api {
namespace impl {
api::CommandBuffer CommandPool_::allocateCommandBuffer()
{
	CommandPool this_ref = static_cast<gles::CommandPoolGles_*>(this)->getReference();
	api::CommandBuffer commandBuffer;
	commandBuffer.construct(m_context, this_ref);
	return commandBuffer;
}

api::SecondaryCommandBuffer CommandPool_::allocateSecondaryCommandBuffer()
{
	CommandPool this_ref = static_cast<gles::CommandPoolGles_*>(this)->getReference();
	api::SecondaryCommandBuffer commandBuffer;
	commandBuffer.construct(m_context, this_ref);
	return commandBuffer;
}
}
}
}