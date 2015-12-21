/*!*********************************************************************************************************************
\file         PVRCore\StackTrace.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief        Functionality to work with Stack Traces. Implemented on specific platforms only.
***********************************************************************************************************************/
#pragma once
#include "PVRCore/CoreIncludes.h"

namespace pvr {
#ifdef _WIN32
#ifdef DEBUG
std::string getStackTraceInfo(int skipFrames);
#else
inline std::string getStackTraceInfo(int skipFrames)
{
	return std::string();
}
#endif
#else
inline std::string getStackTraceInfo(int skipFrames)
{
	return std::string("Stack trace functionality only implemented for Windows Debug builds, sorry.");
}
#endif
}
