#pragma once
/*!*********************************************************************************************************************
\file         PVRShell\ShellData.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Class containing internal data of the PowerVR Shell.
***********************************************************************************************************************/
#include "PVRShell/CommandLine.h"

/*! This file simply defines a version string. It can be commented out. */
#include "sdkver.h"
#if !defined(PVRSDK_VERSION)
#define PVRSDK_VERSION "n.n@nnnnnnn"
#endif

/*! Define the txt file that we can load command-line options from. */
#if !defined(PVRSHELL_COMMANDLINE_TXT_FILE)
#define PVRSHELL_COMMANDLINE_TXT_FILE	"PVRShellCL.txt"
#endif

namespace pvr {
namespace platform {
class ShellOS;

/*!****************************************************************************************************************
\brief  Contains and tracks internal data necessary to power the pvr::Shell.
*******************************************************************************************************************/
struct ShellData
{
	Time timer;
	uint64 timeAtInitApplication;
	uint64 lastFrameTime;
	uint64 currentFrameTime;
	string exitMessage;

	ShellOS* os;
	GraphicsContextStrongReference graphicsContextStore;
	GraphicsContext graphicsContext;// So that we don't have to copy the reference every single get...
	std::auto_ptr<IPlatformContext> platformContext;
	DisplayAttributes attributes;

	platform::CommandLineParser* commandLine;

	int32 captureFrameStart;
	int32 captureFrameStop;
	uint32 captureFrameScale;

	bool trapPointerOnDrag;
	bool forceFrameTime;
	uint32 fakeFrameTime;

	bool presentBackBuffer;
	bool exiting;

	uint32 frameNo;

	bool forceReleaseInitCycle;
	int32 dieAfterFrame;
	float32 dieAfterTime;
	int64 startTime;

	bool outputInfo;

	bool weAreDone;

	float FPS;
	bool showFPS;

	Api contextType;
	Api minContextType;
	DeviceQueueType deviceQueueType;

	ShellData() :	os(0),
		commandLine(0),
		captureFrameStart(-1),
		captureFrameStop(-1),
		captureFrameScale(1),
		trapPointerOnDrag(true),
		forceFrameTime(false),
		fakeFrameTime(16),
		presentBackBuffer(true),
		exiting(false),
		frameNo(0),
		forceReleaseInitCycle(false),
		dieAfterFrame(-1),
		dieAfterTime(-1),
		startTime(0),
		outputInfo(false),
		weAreDone(false),
		FPS(0.0f),
		showFPS(false),
		contextType(Api::Unspecified),
		minContextType(Api::Unspecified),
		deviceQueueType(DeviceQueueType::Graphics)
	{
	};
};
}
}
