/*!*********************************************************************************************************************
\file         PVRCore\OSManager.h
\author       PowerVR by Imagination, Developer Technology Team.
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains the OSManager interface.
***********************************************************************************************************************/
#pragma once
#include "PVRCore/Defines.h"
#include "PVRCore/IPlatformContext.h"

namespace pvr {
namespace VsyncMode {
enum Enum
{
	Off,//!<The application does not synchronizes with the vertical sync. If application renders faster than the display refreshes, frames are wasted and tearing may be observed. FPS is uncapped. Maximum power consumption. If unsupported, "ON" value will be used instead. Minimum latency.
	On,//!<The application is always syncrhonized with the vertical sync. Tearing does not happen. FPS is capped to the display's refresh rate. For fast applications, battery life is improved. Always supported.
	Relaxed,//!<The application synchronizes with the vertical sync, but only if the application rendering speed is greater than refresh rate. Compared to OFF, there is no tearing. Compared to ON, the FPS will be improved for "slower" applications. If unsupported, "ON" value will be used instead. Recommended for most applications. Default if supported.
	Mailbox, //!<The presentation engine will always use the latest fully rendered image. Compared to OFF, no tearing will be observed. Compared to ON, battery power will be worse, especially for faster applications. If unsupported,  "OFF" will be attempted next.
	Half, //!<The application is capped to using half the vertical sync time. FPS artificially capped to Half the display speed (usually 30fps) to maintain battery. Best possible battery savings. Worst possibly performance. Recommended for specific applications where battery saving is critical.
};
};
namespace system {
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

	int32 width;
	int32 height;
	int32 x;
	int32 y;

	int32 depthBPP;
	int32 stencilBPP;

	int32 redBits;
	int32 greenBits;
	int32 blueBits;
	int32 alphaBits;

	int32 aaSamples;

	int32 configID;

	VsyncMode::Enum vsyncMode;
	int32 contextPriority;

	bool forceColorBPP;
	bool fullscreen;
	bool reference;
	bool frameBufferSrgb;

	// Default constructor
	DisplayAttributes() : width(800),
		height(600),
		x(PosDefault),
		y(PosDefault),
		depthBPP(24),
		stencilBPP(0),
		redBits(8),
		greenBits(8),
		blueBits(8),
		alphaBits(8),
		aaSamples(0),
		configID(0),
		vsyncMode(VsyncMode::Relaxed),
		contextPriority(2),
		forceColorBPP(false),
		fullscreen(false),
		reference(false),
		frameBufferSrgb(false)
	{
	}
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
	virtual system::OSDisplay getDisplay() = 0;

	/*!*********************************************************************************************************************
	\brief	GetWindow will return a native handle to the underlying Window of the OS cast into a void*
	***********************************************************************************************************************/
	virtual system::OSWindow getWindow() = 0;

	/*!*********************************************************************************************************************
	\brief	GetWindow will return a native handle to the underlying Window of the OS cast into a void*
	***********************************************************************************************************************/
	virtual IPlatformContext& getPlatformContext() = 0;

	/*!*********************************************************************************************************************
	\brief	GetWindow will return a reference to a DisplayAttributes type containing the configuration of the display.
	***********************************************************************************************************************/
	virtual system::DisplayAttributes& getDisplayAttributes() = 0;

	/*!*********************************************************************************************************************
	\brief	Get the API that is the underlying API of this OS manager.
	***********************************************************************************************************************/
	virtual Api::Enum getApiTypeRequired() = 0;

	/*!*********************************************************************************************************************
	\brief	Get the API that is the underlying API of this OS manager.
	***********************************************************************************************************************/
	virtual Api::Enum getMinApiTypeRequired() = 0;

	/*!*********************************************************************************************************************
	\brief	Set the API type to request. This must be compatible to the PVRApi version linked in.
	***********************************************************************************************************************/
	virtual void setApiTypeRequired(Api::Enum apiType) = 0;

	/*!*********************************************************************************************************************
	\brief	Set the DeviceQueue types requested. This is a bitfield containing the types of devices requested.
	***********************************************************************************************************************/
	virtual DeviceQueueType::Enum getDeviceQueueTypesRequired() = 0;

};

namespace system {
using ::pvr::OSManager;
}
}
