/*!*********************************************************************************************************************
\file         PVRCore\IPlatformContext.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Interface for the class actually performing the low-level operations.
***********************************************************************************************************************/
#pragma once
namespace pvr {

/*!*********************************************************************************************************************
\brief         Enumeration of all types of DeviceQueue.
***********************************************************************************************************************/
namespace DeviceQueueType {
enum Enum
{
	Graphics = 0x01,//<graphics operations
	Compute = 0x02,//< compute operations
	Dma = 0x04,//< DMA operations
	MemoryManagement,//< memory management operations
	Extended = 0x08, //< extended operations
	Count
};
};

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
	\brief release the resources used by this context.
	***********************************************************************************************************************/
	virtual void release() = 0;

	/*!*********************************************************************************************************************
	\brief Initialise this platform context using the functions provided by an OSManager object (e.g. the Shell). Must be called
	       before any other operations are done. Must be called AFTER the IOS manager object that this context belongs to is
		   initialised. Called by the State Machine.
	***********************************************************************************************************************/
	virtual Result::Enum init() = 0;

	/*!*********************************************************************************************************************
	\brief Swap the front and back buffers (called at the end of each frame to show the rendering).
	***********************************************************************************************************************/
	virtual Result::Enum presentBackbuffer() = 0;

	/*!*********************************************************************************************************************
	\brief Bind this context for using.
	***********************************************************************************************************************/
	virtual Result::Enum makeCurrent() = 0;

	/*!*********************************************************************************************************************
	\brief Print information about this context.
	***********************************************************************************************************************/
	virtual std::string getInfo() = 0;

	/*!*********************************************************************************************************************
	\brief Check if this context is initialised.
	***********************************************************************************************************************/
	virtual bool isInitialised() = 0;

	/*!*********************************************************************************************************************
	\brief Get an integer number uniquely identifying this context.
	***********************************************************************************************************************/
	virtual size_t getID() = 0;

	/*!*********************************************************************************************************************
	\brief Get the maximum API version supported by this context.
	***********************************************************************************************************************/
	virtual Api::Enum getMaxApiVersion() = 0;

	/*!*********************************************************************************************************************
	\brief Query if the specified Api is supported by this context.
	***********************************************************************************************************************/
	virtual bool isApiSupported(Api::Enum api) = 0;

	virtual ~IPlatformContext() {}
};

/*!*********************************************************************************************************************
\brief	Allocates and returns a smart pointer to the Platform context depending on the platform. Implemented in the specific
        PVRPlatformGlue to return the correct type of context required.
***********************************************************************************************************************/
std::auto_ptr<IPlatformContext> createNativePlatformContext(OSManager& osManager);

/*!*********************************************************************************************************************
\brief	Performs one-time-only initialisation for the platform context. Implemented in the specific PVRPlatformGlue.
***********************************************************************************************************************/
void initialiseNativeContext();

/*!*********************************************************************************************************************
\brief	Performs one-time-only resource release for the platform context. Implemented in the specific PVRPlatformGlue.
***********************************************************************************************************************/
void releaseNativeContext();

}// namespace pvr