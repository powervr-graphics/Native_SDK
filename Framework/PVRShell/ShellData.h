/*!
\brief Class containing internal data of the PowerVR Shell.
\file PVRShell/ShellData.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRShell/CommandLine.h"
#include "PVRCore/Base/ComplexTypes.h"

/*! This file simply defines a version std::string. It can be commented out. */
#include "sdkver.h"
#if !defined(PVRSDK_BUILD)
#define PVRSDK_BUILD "n.n@nnnnnnn"
#endif

/*! Define the txt file that we can load command-line options from. */
#if !defined(PVRSHELL_COMMANDLINE_TXT_FILE)
#define PVRSHELL_COMMANDLINE_TXT_FILE "PVRShellCL.txt"
#endif

namespace pvr {
namespace platform {
class ShellOS;

/// <summary>Internal. Contains and tracks internal data necessary to power the pvr::Shell.</summary>
struct ShellData
{
	//!\cond NO_DOXYGEN
	Time timer;
	uint64_t timeAtInitApplication;
	uint64_t lastFrameTime;
	uint64_t currentFrameTime;
	std::string exitMessage;

	ShellOS* os;
	DisplayAttributes attributes;

	CommandLineParser* commandLine;

	int32_t captureFrameStart;
	int32_t captureFrameStop;
	uint32_t captureFrameScale;

	bool trapPointerOnDrag;
	bool forceFrameTime;
	uint32_t fakeFrameTime;

	bool exiting;

	uint32_t frameNo;

	bool forceReleaseInitCycle;
	int32_t dieAfterFrame;
	float dieAfterTime;
	int64_t startTime;

	bool outputInfo;

	bool weAreDone;

	float FPS;
	bool showFPS;

	Api contextType;
	Api minContextType;
	ShellData() : os(0),
		commandLine(0),
		captureFrameStart(-1),
		captureFrameStop(-1),
		captureFrameScale(1),
		trapPointerOnDrag(true),
		forceFrameTime(false),
		fakeFrameTime(16),
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
		minContextType(Api::Unspecified)
	{
	};

	//!\endcond
};
}
}
