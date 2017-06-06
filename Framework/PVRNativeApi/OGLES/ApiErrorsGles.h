/*!
\brief Convenience functions for automatically logging API errors. Some functions NOP on release builds.
\file PVRNativeApi/OGLES/ApiErrorsGles.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/Log.h"
namespace pvr {
namespace nativeGles {
/// <summary>Check and return api error.</summary>
/// <param name="errOutStr">error string to be output.</param>
/// <returns>api error code</returns>
int checkApiError(string* errOutStr = NULL);

/// <summary>Check for API errors in APIs that support this Log api error, CHECK_API_ERROR must be defined,
/// otherwise this function has no effect.</summary>
/// <param name="severity">that the error will be reported with, if an error is found</param>
/// <param name="note">A c-style string that will be prepended to the error description if an error is found.
/// </param>
/// <returns>true if an API error has occured, false otherwise</returns>
bool logApiError(const char* note, Logger::Severity severity = Logger::Error);

/// <summary>Checks if the provided result code is successful, log possible api errors if not.</summary>
/// <param name="res">A return code from a function.</param>
/// <returns>true if res is successful errors.</returns>
}
}

#ifdef DEBUG
/// <summary>Checks for API errors if the API supports them. If an error is detected, logs relevant error
/// information. Only works in debug builds, and compiles to a NOP in release builds.</summary>
/// <param name="note">A note that will be prepended to the error log, if an error is detected.</param>
#define debugLogApiError(note) ::pvr::nativeGles::logApiError(note)
#else
/// <summary>Checks for API errors if the API supports them. If an error is detected, logs relevant error
/// information. Only works in debug builds, and compiles to a NOP in release builds.</summary>
#define debugLogApiError(dummy)
#endif