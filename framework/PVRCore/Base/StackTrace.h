/*!
\brief Functionality to work with Stack Traces. Implemented on specific platforms only.
\file PVRCore/Base/StackTrace.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/CoreIncludes.h"

namespace pvr {
#ifdef _WIN32
#ifdef DEBUG
/// <summary>Get the stacktrace as a string, skipping the specified number of stack frames from the top of the stack.
/// **ONLY IMPLEMENTED FOR 32 bit WINDOWS DEBUG BUILDS**</summary>
/// <param name="skipFrames">The number of frames to skip</param>
/// <returns>The stack trace</returns>
std::string getStackTraceInfo(int skipFrames);
#else
/// <summary>Get the stacktrace as a string, skipping the specified number of stack frames from the top of the stack.
/// **ONLY IMPLEMENTED FOR 32 bit WINDOWS DEBUG BUILDS**</summary>
/// <param name="skipFrames">The number of frames to skip</param>
/// <returns>The stack trace</returns>
inline std::string getStackTraceInfo(int skipFrames)
{
	return std::string();
}
#endif
#else
/// <summary>Get the stacktrace as a string, skipping the specified number of stack frames from the top of the stack.
/// **ONLY IMPLEMENTED FOR 32 bit WINDOWS DEBUG BUILDS**</summary>
/// <param name="skipFrames">The number of frames to skip</param>
/// <returns>The stack trace</returns>
inline std::string getStackTraceInfo(int skipFrames)
{
	return std::string("Stack trace functionality only implemented for Windows Debug builds, sorry.");
}
#endif
}