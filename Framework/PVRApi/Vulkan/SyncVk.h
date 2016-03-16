/*!*********************************************************************************************************************
\file         PVRApi/Vulkan/SyncVk.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Contains Vulkan specific implementation of the Fence class. Use only if directly using Vulkan calls.
			  Provides the definitions allowing to move from the Framework object Fence to the underlying Vulkan Fence.
***********************************************************************************************************************/
#pragma once
#include "PVRApi/ApiObjects/Sync.h"
#include "PVRNativeApi/Vulkan/NativeObjectsVk.h"
#include "PVRNativeApi/Vulkan/VulkanBindings.h"
#include "PVRNativeApi/Vulkan/ConvertToVkTypes.h"
#include "PVRApi/Vulkan/ContextVk.h"

namespace pvr {
namespace api {
namespace vulkan {
/*!*********************************************************************************************************************
\brief Vulkan implementation of the Fence class.
***********************************************************************************************************************/
class FenceVk_ : public impl::Fence_, public native::HFence_
{
public:
	/*!*********************************************************************************************************************
	\brief ctor. Construct this object
	\param context The Context to be construct from
	\param fence A native handle to the fence that need to be wrapped in 
	***********************************************************************************************************************/
	FenceVk_(const GraphicsContext& context, HFence_ fence) : Fence_(context), HFence_(fence){}
	
	/*!*********************************************************************************************************************
	\brief ctor. Construct this object
	\param context The Context to be construct from
	***********************************************************************************************************************/
	FenceVk_(const GraphicsContext& context) : Fence_(context){}
	
	/*!*********************************************************************************************************************
	\brief dtor.
	***********************************************************************************************************************/
	virtual ~FenceVk_();
	
	/*!*********************************************************************************************************************
	\brief Initialize this object.
	\param createSignaled Create this Fence with the signaled state
	***********************************************************************************************************************/
	bool init(bool createSignaled);
	
	/*!*********************************************************************************************************************
	\brief Destroy this object.
	***********************************************************************************************************************/
	void destroy();
};

/*!*********************************************************************************************************************
\brief Vulkan implementation of the Semaphore class.
***********************************************************************************************************************/
class SemaphoreVk_ : public impl::Semaphore_, public native::HSemaphore_
{
	/*!*********************************************************************************************************************
	\brief ctor. Construct this object
	\param context The Context to be construct from
	\param A native handle to the semaphore that need to be wrapped in to
	***********************************************************************************************************************/
	SemaphoreVk_(const GraphicsContext& context, HSemaphore_ semaphore) : Semaphore_(context), HSemaphore_(semaphore){}
	
	/*!*********************************************************************************************************************
	\brief ctor. Construct this object
	\param context The Context to be construct from
	***********************************************************************************************************************/
	SemaphoreVk_(const GraphicsContext& context) : Semaphore_(context) { }
	
	/*!*********************************************************************************************************************
	\brief dtor.
	***********************************************************************************************************************/
	virtual ~SemaphoreVk_();
	
	/*!*********************************************************************************************************************
	\brief Initialize this object.
	***********************************************************************************************************************/
	bool init();
	
	/*!*********************************************************************************************************************
	\brief Destroy this object.
	***********************************************************************************************************************/
	void destroy();
};

/*!*********************************************************************************************************************
\brief Vulkan implementation of the Event class.
***********************************************************************************************************************/
class EventVk_ : public impl::Event_, public native::HEvent_
{
	/*!*********************************************************************************************************************
	\brief ctor. Construct this object
	\param context The Context to be construct from
	***********************************************************************************************************************/
	EventVk_(const GraphicsContext& context, HEvent_ event) : Event_(context), HEvent_(event){}
	
	/*!*********************************************************************************************************************
	\brief ctor. Construct this object
	\param context The Context to be construct from
	***********************************************************************************************************************/
	EventVk_(const GraphicsContext& context) : Event_(context) { }
	
	/*!*********************************************************************************************************************
	\brief dtor.
	***********************************************************************************************************************/
	virtual ~EventVk_();
	
	/*!*********************************************************************************************************************
	\brief Initialize this object.
	\param createSignaled Create this Fence with the signaled state
	***********************************************************************************************************************/
	bool init();
	
	/*!*********************************************************************************************************************
	\brief Destroy this object.
	***********************************************************************************************************************/
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

