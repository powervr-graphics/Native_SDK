/*!
\brief Convenience functions for automatically logging API errors. Some functions NOP on release builds.
\file PVRUtils/OGLES/ErrorsGles.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/Log.h"
namespace pvr {
namespace utils {
/// <summary>Checks and returns api error if appropriate.</summary>
/// <param name="errOutStr">error std::string to be output.</param>
/// <returns>api error code</returns>
int checkApiError(std::string* errOutStr = NULL);

/// <summary>Checks and logs api errors if appropriate.</summary>
/// <param name="note">A c-style std::string that will be prepended to the error description if an error is found.</param>
/// <param name="severity">that the error will be reported with, if an error is found</param>
/// <returns>true if an API error has occured, false otherwise</returns>
bool logApiError(const char* note, LogLevel severity = LogLevel::Error);
}
}

#ifdef DEBUG
/// <summary>Checks for API errors if the API supports them. If an error is detected, logs relevant error
/// information. Only works in debug builds, and compiles to a NOP in release builds.</summary>
/// <param name="note">A note that will be prepended to the error log, if an error is detected.</param>
#define debugLogApiError(note) ::pvr::utils::logApiError(note)
#else
/// <summary>Checks for API errors if the API supports them. If an error is detected, logs relevant error
/// information. Only works in debug builds, and compiles to a NOP in release builds.</summary>
#define debugLogApiError(dummy)
#endif