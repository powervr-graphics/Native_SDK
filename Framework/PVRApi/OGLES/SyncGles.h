/*!
\brief Contains OpenGLES specific implementation of the synchronization classes (Fence, Semaphore,
MemoryBarrier. Use only if directly using Vulkan calls. Provides
the definitions allowing to move from the Framework object Fence to the underlying Vulkan Fence.
\file PVRApi/Vulkan/SyncVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRApi/ApiObjects/Sync.h"
#include "PVRNativeApi/OGLES/NativeObjectsGles.h"
#include "PVRNativeApi/OGLES/OpenGLESBindings.h"
#include "PVRNativeApi/OGLES/ConvertToApiTypes.h"
#include "PVRApi/OGLES/ContextGles.h"

namespace pvr {
namespace api {
namespace gles {
/// <summary>Vulkan implementation of the Fence class.</summary>
class FenceGles_ : public impl::Fence_, public native::HFence_
{
public:
	/// <summary>ctor. Construct this object</summary>
	/// <param name="context">The Context to be construct from</param>
	FenceGles_(const GraphicsContext& context, native::HFence_ fence) : Fence_(context), HFence_(fence) {}

private:
	bool wait_(uint64 timeoutNanos)
	{
		GLenum res = gl::ClientWaitSync(handle, GL_SYNC_FLUSH_COMMANDS_BIT, timeoutNanos);
		return res == GL_ALREADY_SIGNALED || res == GL_CONDITION_SATISFIED;
	}
	bool isSignalled_()
	{
		GLenum res = gl::ClientWaitSync(handle, GL_SYNC_FLUSH_COMMANDS_BIT, 0);
		return res == GL_ALREADY_SIGNALED || res == GL_CONDITION_SATISFIED;
	}
	void reset_() {}
};

/// <summary>Vulkan implementation of the Semaphore class.</summary>
class SemaphoreGles_ : public impl::Semaphore_, public native::HSemaphore_
{
public:
	/// <summary>ctor. Construct this object</summary>
	/// <param name="context">The Context to be construct from</param>
	SemaphoreGles_(const GraphicsContext& context) : Semaphore_(context) { }
};

/// <summary>Vulkan implementation of the Event class.</summary>
class EventGles_ : public impl::Event_, public native::HEvent_
{
	/// <summary>ctor. Construct this object</summary>
	/// <param name="context">The Context to be construct from</param>
	EventGles_(const GraphicsContext& context) : Event_(context) { }

};

typedef RefCountedResource<EventGles_> EventGles;
typedef RefCountedResource<SemaphoreGles_> SemaphoreGles;
typedef RefCountedResource<FenceGles_> FenceGles;
}//namespace vulkan
}//namespace api
}//namespace pvr
