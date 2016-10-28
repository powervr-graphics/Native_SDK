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

    /*!
       \brief Allocate a primary commandBuffer
       \return Return a valid commandbuffer if allocation success
     */
	api::CommandBuffer allocateCommandBuffer();

    /*!
       \brief Allocate a secondary commandBuffer
       \return Return a valid commandbuffer if allocation success
     */
	api::SecondaryCommandBuffer allocateSecondaryCommandBuffer();

    /*!
       \brief Return a handle to the native object
       \return
     */
	native::HCommandPool_& getNativeObject();

    /*!
       \brief Return a handle to native object (const)
     */
	const native::HCommandPool_& getNativeObject()const;
protected:
	CommandPool_(const GraphicsContext& context) : m_context(context) {}
	GraphicsContext m_context;
};
}
}
}
