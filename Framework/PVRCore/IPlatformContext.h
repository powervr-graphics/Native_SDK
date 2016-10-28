/*!*********************************************************************************************************************
\file         PVRCore\IPlatformContext.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Interface for the class actually performing the low-level operations.
***********************************************************************************************************************/
#pragma once
#include <algorithm>
namespace pvr {

/*!*********************************************************************************************************************
\brief         Enumeration of all types of DeviceQueue.
***********************************************************************************************************************/
enum class DeviceQueueType
{
	Graphics = 0x01,//<graphics operations
	Compute = 0x02,//< compute operations
	Dma = 0x04,//< DMA operations
	Extended    = 0x08, //< extended operations
	MemoryManagement,//< memory management operations
	Count
};

inline DeviceQueueType operator&(DeviceQueueType a, DeviceQueueType b)
{
	return DeviceQueueType(std::underlying_type<DeviceQueueType>::type(a) &
	                       std::underlying_type<DeviceQueueType>::type(b));
}

inline DeviceQueueType operator|(DeviceQueueType a, DeviceQueueType b)
{
	return DeviceQueueType(std::underlying_type<DeviceQueueType>::type(a) |
	                       std::underlying_type<DeviceQueueType>::type(b));
}

namespace platform {
struct NativePlatformHandles_; struct NativeDisplayHandle_;
}

class OSManager;
/*!*********************************************************************************************************************
\brief Interface for the Platform context. Contains the necessary operations needed to get the context required to create a window,
       swap the buffers, and make current. Has an ID used to identify it.
	   Specific platform contexts (EglPlatformContext, VulkanPlatformContext) implement this interface.
***********************************************************************************************************************/
class IPlatformContext
{
public:
	/*!*********************************************************************************************************************
	\brief Swap the front and back buffers (called at the end of each frame to show the rendering).
	***********************************************************************************************************************/
	virtual Result init() = 0;

	/*!*********************************************************************************************************************
	\brief Swap the front and back buffers (called at the end of each frame to show the rendering).
	***********************************************************************************************************************/
	virtual void release() = 0;

	/*!*********************************************************************************************************************
	\brief Swap the front and back buffers (called at the end of each frame to show the rendering).
	***********************************************************************************************************************/
	virtual bool presentBackbuffer() = 0;

	/*!*********************************************************************************************************************
	\brief Bind this context for using.
	***********************************************************************************************************************/
	virtual bool makeCurrent() = 0;

	/*!*********************************************************************************************************************
	\brief Print information about this context.
	***********************************************************************************************************************/
	virtual std::string getInfo() = 0;

	/*!*********************************************************************************************************************
	\brief Check if this context is initialized.
	***********************************************************************************************************************/
	virtual bool isInitialized() const = 0;

	/*!*********************************************************************************************************************
	\brief Get an integer number uniquely identifying this context.
	***********************************************************************************************************************/
	virtual size_t getID() const = 0;

	/*!*********************************************************************************************************************
	\brief Get the maximum API version supported by this context.
	***********************************************************************************************************************/
	virtual Api getMaxApiVersion() = 0;

	/*!*********************************************************************************************************************
	\brief Get the maximum API version supported by this context.
	***********************************************************************************************************************/
	Api getApiType() { return apiType; }

	/*!*********************************************************************************************************************
	\brief Query if the specified Api is supported by this context.
	***********************************************************************************************************************/
	virtual bool isApiSupported(Api api) = 0;

	/*!*********************************************************************************************************************
	\brief Get the NativePlatformHandles wrapped by this context.
	***********************************************************************************************************************/
	virtual const platform::NativePlatformHandles_& getNativePlatformHandles() const = 0;

	/*!*********************************************************************************************************************
	\brief Get the NativePlatformHandles wrapped by this context.
	***********************************************************************************************************************/
	virtual platform::NativePlatformHandles_& getNativePlatformHandles() = 0;

	/*!*********************************************************************************************************************
	\brief Get the NativePlatformHandles wrapped by this context.
	***********************************************************************************************************************/
	virtual const platform::NativeDisplayHandle_& getNativeDisplayHandle() const = 0;

	/*!*********************************************************************************************************************
	\brief Get the NativePlatformHandles wrapped by this context.
	***********************************************************************************************************************/
	virtual platform::NativeDisplayHandle_& getNativeDisplayHandle() = 0;

	/*!*********************************************************************************************************************
	\brief Get the NativePlatformHandles wrapped by this context.
	***********************************************************************************************************************/
	virtual uint32 getSwapChainLength() const = 0;

	/*!*********************************************************************************************************************
	\brief Get the NativePlatformHandles wrapped by this context.
	***********************************************************************************************************************/
	uint32 getSwapChainIndex() const { return swapIndex; }

	uint32 getLastSwapChainIndex()const { return lastPresentedSwapIndex; }

	IPlatformContext() : swapIndex(0), lastPresentedSwapIndex(0), apiType(Api::Unspecified) {}

	virtual ~IPlatformContext() { }
//protected:
	uint32      swapIndex;
	uint32      lastPresentedSwapIndex;
	Api   apiType;
};

/*!*********************************************************************************************************************
\brief	Allocates and returns a smart pointer to the Platform context depending on the platform. Implemented in the specific
        PVRPlatformGlue to return the correct type of context required.
***********************************************************************************************************************/
std::auto_ptr<IPlatformContext> createNativePlatformContext(OSManager& osManager);

}// namespace pvr
