/*!
\brief Contains Vulkan specific implementation of the Fence class. Use only if directly using Vulkan calls. Provides
the definitions allowing to move from the Framework object Fence to the underlying Vulkan Fence.
\file PVRApi/Vulkan/SyncVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRApi/ApiObjects/Sync.h"
#include "PVRNativeApi/Vulkan/NativeObjectsVk.h"
#include "PVRNativeApi/Vulkan/VulkanBindings.h"
#include "PVRNativeApi/Vulkan/ConvertToVkTypes.h"
#include "PVRApi/Vulkan/ContextVk.h"

namespace pvr {
namespace api {
namespace vulkan {
/// <summary>Vulkan implementation of the Fence class.</summary>
class FenceVk_ : public impl::Fence_, public native::HFence_
{
public:
	/// <summary>ctor. Construct this object</summary>
	/// <param name="context">The Context to be construct from</param>
	/// <param name="fence">A native handle to the fence that need to be wrapped in</param>
	FenceVk_(const GraphicsContext& context, HFence_ fence) : Fence_(context), HFence_(fence){}
	
	/// <summary>ctor. Construct this object</summary>
	/// <param name="context">The Context to be construct from</param>
	FenceVk_(const GraphicsContext& context) : Fence_(context){}
	
	/// <summary>dtor.</summary>
	virtual ~FenceVk_();
	
	/// <summary>Initialize this object.</summary>
	/// <param name="createSignaled">Create this Fence with the signaled state</param>
	bool init(bool createSignaled);
	
	/// <summary>Destroy this object.</summary>
	void destroy();

	bool wait_(uint64 timeoutNanos);
	bool isSignalled_();
	void reset_();
};

/// <summary>Vulkan implementation of the Semaphore class.</summary>
class SemaphoreVk_ : public impl::Semaphore_, public native::HSemaphore_
{
public:
	/// <summary>ctor. Construct this object</summary>
	/// <param name="context">The Context to be construct from</param>
	/// <param name="A">native handle to the semaphore that need to be wrapped in to</param>
	SemaphoreVk_(const GraphicsContext& context, HSemaphore_ semaphore) : Semaphore_(context), HSemaphore_(semaphore){}
	
	/// <summary>ctor. Construct this object</summary>
	/// <param name="context">The Context to be construct from</param>
	SemaphoreVk_(const GraphicsContext& context) : Semaphore_(context) { }
	
	/// <summary>dtor.</summary>
	virtual ~SemaphoreVk_();
	
	/// <summary>Initialize this object.</summary>
	bool init();
	
	/// <summary>Destroy this object.</summary>
	void destroy();
};

/// <summary>Vulkan implementation of the Event class.</summary>
class EventVk_ : public impl::Event_, public native::HEvent_
{
	/// <summary>ctor. Construct this object</summary>
	/// <param name="context">The Context to be construct from</param>
	EventVk_(const GraphicsContext& context, HEvent_ event) : Event_(context), HEvent_(event){}
	
	/// <summary>ctor. Construct this object</summary>
	/// <param name="context">The Context to be construct from</param>
	EventVk_(const GraphicsContext& context) : Event_(context) { }
	
	/// <summary>dtor.</summary>
	virtual ~EventVk_();
	
	/// <summary>Initialize this object.</summary>
	/// <param name="createSignaled">Create this Fence with the signaled state</param>
	bool init();
	
	/// <summary>Destroy this object.</summary>
	void destroy();
};

typedef RefCountedResource<EventVk_> EventVk;
typedef RefCountedResource<SemaphoreVk_> SemaphoreVk;
typedef RefCountedResource<FenceVk_> FenceVk;
}//namespace vulkan
}//namespace api
}//namespace pvr

PVR_DECLARE_NATIVE_CAST(Event);
PVR_DECLARE_NATIVE_CAST(Fence);
PVR_DECLARE_NATIVE_CAST(Semaphore);
