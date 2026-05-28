/*!
\brief Logging helper function
\file PVRSuperResolution/Log.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

#include <vector>
#include "../include/vulkan/vulkan.h"

namespace pvr {

/// <summary>Checks whether a debugger can be found for the current running process (on Windows and Linux only).
/// The prescene of a debugger can be used to provide additional helpful functionality for debugging application issues one of which could be to break in the
/// debugger when an exception is thrown. Being able to have the debugger break on such a thrown exception provides by far the most seamless and constructive environment for
/// fixing an issue causing the exception to be thrown due to the full state and stack trace being present at the point in which the issue has occurred rather
/// than relying on error logic handling.</summary>
/// <returns>True if a debugger can be found for the current running process else False.</returns>
bool isDebuggerPresent();

/// <summary>Print a message given by parameter.</summary>
/// <param name="formatString">String to print.</param>
void logMessage(const char* formatString);

/// <summary>Throw an assert if condition parameter is false.</summary>
/// <param name="condition">Condition to throw the assert.</param>
/// <param name="msg">Print message.</param>
void assertCondition(bool condition, const char* msg);

/// <summary>Throw an assert if the specified result is not VK_SUCCESS.
/// <param name="result">A Vulkan result code.</param>
/// <param name="msg">Print msg if the error code is not VK_SUCCESS nor VK_SUBOPTIMAL_KHR.</param>
void assertFunctionResult(VkResult result, const char* msg);

} // namespace pvr
