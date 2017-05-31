/*!
\brief Definition of the CommandPool class.
\file PVRApi/ApiObjects/CommandPool.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRApi/ApiIncludes.h"
#include "PVRApi/ApiObjects/CommandBuffer.h"
namespace pvr {
namespace api {
namespace impl {
/// <summary>A Pool object that is used to allocate command buffer. Create a Command pool 
/// object to allocate command buffers in threads other than the main thread. Otherwise you
/// can use the Context's default Command Pool.</summary>
class CommandPool_
{
public:
	/// <summary> Destructor.</summary>
	virtual ~CommandPool_() { }
	GraphicsContext& getContext() { return  _context; }

    /// <summary>Allocate a primary commandBuffer</summary>
    /// <returns>Return a valid commandbuffer if allocation is successful, otherwise a null CommandBuffer</returns>
	api::CommandBuffer allocateCommandBuffer();

    /// <summary>Allocate a secondary commandBuffer</summary>
    /// <returns>Return a valid commandbuffer if allocation success, otherwise a null CommandBuffer</returns>
	api::SecondaryCommandBuffer allocateSecondaryCommandBuffer();

protected:
	CommandPool_(const GraphicsContext& context) : _context(context) {}
	GraphicsContext _context;
};
}
}
}