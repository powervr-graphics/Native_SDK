#pragma once
#include "PVRApi/ApiIncludes.h"
#include "PVRApi/ApiObjects/CommandBuffer.h"
namespace pvr {
namespace api {
namespace impl {
class CommandPool_
{
public:
	virtual ~CommandPool_() { }
	GraphicsContext& getContext() { return  m_context; }

	api::CommandBuffer allocateCommandBuffer();

	api::SecondaryCommandBuffer allocateSecondaryCommandBuffer();

	native::HCommandPool_& getNativeObject();
	const native::HCommandPool_& getNativeObject()const;
protected:
	CommandPool_(const GraphicsContext& context) : m_context(context) {}
	GraphicsContext m_context;
};
}
}
}
