/*!*********************************************************************************************************************
\file         PVRCore\OSManager.h
\author       PowerVR by Imagination, Developer Technology Team.
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains the OSManager interface.
***********************************************************************************************************************/
#pragma once
#include "PVRCore/Defines.h"
#include "PVRCore/IPlatformContext.h"
#include "PVRCore/IAssetProvider.h"

namespace pvr {
class IGraphicsContext;
typedef pvr::RefCountedWeakReference<IGraphicsContext> GraphicsContext;

enum class VsyncMode
{
	Off,//!<The application does not synchronizes with the vertical sync. If application renders faster than the display refreshes, frames are wasted and tearing may be observed. FPS is uncapped. Maximum power consumption. If unsupported, "ON" value will be used instead. Minimum latency.
	On,//!<The application is always syncrhonized with the vertical sync. Tearing does not happen. FPS is capped to the display's refresh rate. For fast applications, battery life is improved. Always supported.
	Relaxed,//!<The application synchronizes with the vertical sync, but only if the application rendering speed is greater than refresh rate. Compared to OFF, there is no tearing. Compared to ON, the FPS will be improved for "slower" applications. If unsupported, "ON" value will be used instead. Recommended for most applications. Default if supported.
	Mailbox, //!<The presentation engine will always use the latest fully rendered image. Compared to OFF, no tearing will be observed. Compared to ON, battery power will be worse, especially for faster applications. If unsupported,  "OFF" will be attempted next.
	Half, //!<The application is capped to using half the vertical sync time. FPS artificially capped to Half the display speed (usually 30fps) to maintain battery. Best possible battery savings. Worst possibly performance. Recommended for specific applications where battery saving is critical.
};

namespace platform {
/*!*********************************************************************************************************************
\brief        Contains display configuration information (width, height, position, title, bpp etc.).
***********************************************************************************************************************/
class DisplayAttributes
{
public:
	enum
	{
		PosDefault = -1
	};

	std::string  windowTitle;

	uint32 width;
	uint32 height;
	uint32 x;
	uint32 y;

	uint32 depthBPP;
	uint32 stencilBPP;

	uint32 redBits;
	uint32 greenBits;
	uint32 blueBits;
	uint32 alphaBits;

	uint32 aaSamples;

	uint32 configID;

	VsyncMode vsyncMode;
	int32 contextPriority;
	int32 swapLength;

	bool forceColorBPP;
	bool fullscreen;
	bool reference;
	bool frameBufferSrgb;

	// Default constructor
    DisplayAttributes() :
        width(800u),
		height(600u),
		x((uint32)PosDefault),
		y((uint32)PosDefault),
		depthBPP(32u),
		stencilBPP(0u),
		redBits(8u),
		greenBits(8u),
		blueBits(8u),
		alphaBits(8u),
		aaSamples(0u),
		configID(0u),
		vsyncMode(VsyncMode::On),
		contextPriority(2),
        swapLength(3),
		forceColorBPP(false),
		fullscreen(false),
		reference(false),
        frameBufferSrgb(false)
	{
	}
    bool isScreenRotated()const{ return height > width; }
};

/*!*********************************************************************************************************************
\brief        Native display type.
***********************************************************************************************************************/
typedef void* OSDisplay;

/*!*********************************************************************************************************************
\brief        Native window type.
***********************************************************************************************************************/
typedef void* OSWindow;

/*!*********************************************************************************************************************
\brief        Native application type.
***********************************************************************************************************************/
typedef void* OSApplication;

/*!*********************************************************************************************************************
\brief        Native application data type.
***********************************************************************************************************************/
typedef void* OSDATA;
}

/*!*********************************************************************************************************************
\brief	This interface abstracts the part of the Shell that will provide the Display and the Window so that the context
		can be initialized.
***********************************************************************************************************************/
class OSManager
{
public:
	/*!*********************************************************************************************************************
	\brief	GetDisplay will return a native handle to the underlying Display of the OS cast into a void*
	***********************************************************************************************************************/
	virtual platform::OSDisplay getDisplay() = 0;

	/*!*********************************************************************************************************************
	\brief	GetWindow will return a native handle to the underlying Window of the OS cast into a void*
	***********************************************************************************************************************/
	virtual platform::OSWindow getWindow() = 0;

	/*!*********************************************************************************************************************
	\brief	GetWindow will return a native handle to the underlying Window of the OS cast into a void*
	***********************************************************************************************************************/
	virtual IPlatformContext& getPlatformContext() = 0;

	/*!*********************************************************************************************************************
	\brief	GetWindow will return a reference to a DisplayAttributes type containing the configuration of the display.
	***********************************************************************************************************************/
	virtual platform::DisplayAttributes& getDisplayAttributes() = 0;

	/*!*********************************************************************************************************************
	\brief	Get the API that is the underlying API of this OS manager.
	***********************************************************************************************************************/
	virtual Api getApiTypeRequired() = 0;

	/*!*********************************************************************************************************************
	\brief	Get the API that is the underlying API of this OS manager.
	***********************************************************************************************************************/
	virtual Api getMinApiTypeRequired() = 0;

	/*!*********************************************************************************************************************
	\brief	Set the API type to request. This must be compatible to the PVRApi version linked in.
	***********************************************************************************************************************/
	virtual void setApiTypeRequired(Api apiType) = 0;

	/*!*********************************************************************************************************************
	\brief	Set the DeviceQueue types requested. This is a bitfield containing the types of devices requested.
	***********************************************************************************************************************/
	virtual DeviceQueueType getDeviceQueueTypesRequired() = 0;

	/*!*********************************************************************************************************************
	\brief    Return the main GraphicsContext of this AssetProvider
	\return   The main GraphicsContext of this AssetProvider
	***********************************************************************************************************************/
	virtual GraphicsContext& getGraphicsContext() = 0;
};

class IPlatformProvider: public OSManager, public IAssetProvider
{
};

namespace platform {
using ::pvr::OSManager;
}
}
