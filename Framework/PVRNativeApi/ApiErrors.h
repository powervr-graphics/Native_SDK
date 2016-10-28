/*!*********************************************************************************************************************
\file         PVRNativeApi\ApiErrors.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Convenience functions for automatically logging API errors. Some functions NOP on release builds.
***********************************************************************************************************************/
#include "PVRCore/Log.h"
#pragma once
namespace pvr {
namespace api {
/*!****************************************************************************************************************
\brief Check and return api error.
\param errOutStr error string to be output.
\return api error code
******************************************************************************************************************/
int checkApiError(string* errOutStr = NULL);

/*!****************************************************************************************************************
\brief  Check for API errors in APIs that support this Log api error, CHECK_API_ERROR must be defined, otherwise
        this function has no effect.
\param  severity that the error will be reported with, if an error is found
\param  note A c-style string that will be prepended to the error description if an error is found.
\return true if an API error has occured, false otherwise
*******************************************************************************************************************/
bool logApiError(const char* note, Logger::Severity severity = Logger::Error);

/*!****************************************************************************************************************
\brief  Checks if the provided result code is successful, log possible api errors if not.
\param  res A return code from a function.
\return true if res is successful errors.
*******************************************************************************************************************/
bool succeeded(Result res);

#ifdef DEBUG
/*!****************************************************************************************************************
\brief  Checks for API errors if the API supports them. If an error is detected, logs relevant error information.
        Only works in debug builds, and compiles to a NOP in release builds.
\param  note A note that will be prepended to the error log, if an error is detected.
*******************************************************************************************************************/
inline void debugLogApiError(const char* note)
{
	logApiError(note);
}
#else
/*!****************************************************************************************************************
\brief  Checks for API errors if the API supports them. If an error is detected, logs relevant error information.
        Only works in debug builds, and compiles to a NOP in release builds.
*******************************************************************************************************************/
inline void debugLogApiError(const char* /*note*/)
{
	return;
}
#endif
}
}
