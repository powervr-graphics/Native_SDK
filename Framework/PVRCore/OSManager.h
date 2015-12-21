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

	int32 swapInterval;
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
		swapInterval(1),
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
		can be initialised.
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
