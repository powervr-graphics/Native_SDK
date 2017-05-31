/*!
\brief OpenGL ES Implementation details of the CommandPool class.
\file PVRApi/OGLES/CommandPoolGles.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRApi/OGLES/CommandPoolGles.h"
#include "PVRApi/OGLES/CommandBufferGles.h"
namespace pvr {
namespace api {
namespace impl {
api::CommandBuffer CommandPool_::allocateCommandBuffer()
{

	CommandPool this_ref = static_cast<gles::CommandPoolGles_*>(this)->getReference();
	auto* pimpl_es = new api::impl::CommandBufferImplGles_(_context, this_ref);
	std::auto_ptr<api::impl::ICommandBufferImpl_> pimpl(pimpl_es);
	api::CommandBuffer commandBuffer;
	commandBuffer.construct(pimpl);
	pimpl_es->_myowner = &*commandBuffer;
	return commandBuffer;
}

api::SecondaryCommandBuffer CommandPool_::allocateSecondaryCommandBuffer()
{
	CommandPool this_ref = static_cast<gles::CommandPoolGles_*>(this)->getReference();
	auto* pimpl_es = new api::impl::CommandBufferImplGles_(_context, this_ref);
	std::auto_ptr<api::impl::ICommandBufferImpl_> pimpl(pimpl_es);
	api::SecondaryCommandBuffer commandBuffer;
	commandBuffer.construct(pimpl);
	pimpl_es->_myowner = &*commandBuffer;
	return commandBuffer;
}
}
}
}
