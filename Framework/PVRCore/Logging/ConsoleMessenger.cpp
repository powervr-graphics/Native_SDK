/*!
\brief Implementations of methods of ConsoleMessenger class.
\file PVRCore/Logging/ConsoleMessenger.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include <cstdlib>
#include <cstdarg>
#include <cstring>
#include <cstdio>
#include "PVRCore/Logging/ConsoleMessenger.h"

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#define vsnprintf _vsnprintf
#endif

#if defined(__ANDROID__)
#include <android/log.h>
namespace pvr {
static const android_LogPriority messageTypes[] =
{
	ANDROID_LOG_VERBOSE,
	ANDROID_LOG_DEBUG,
	ANDROID_LOG_INFO,
	ANDROID_LOG_WARN,
	ANDROID_LOG_ERROR,
	ANDROID_LOG_FATAL,
};
}
#elif defined(__QNXNTO__)
#include <sys/slog.h>
namespace pvr {
static const int messageTypes[] =
{
	_SLOG_DEBUG1,
	_SLOG_DEBUG1,
	_SLOG_INFO,
	_SLOG_WARNING,
	_SLOG_ERROR,
	_SLOG_CRITICAL
};
}
#else
namespace {
static const pvr::char8* messageTypes[] =
{
	"VERBOSE: ",
	"DEBUG: ",
	"INFORMATION: ",
	"WARNING: ",
	"ERROR: ",
	"CRITICAL: ",
};
}
#endif
namespace pvr {
namespace platform {
void ConsoleMessenger::initializeMessenger()
{
#if defined(PVR_PLATFORM_IS_DESKTOP) && !defined(TARGET_OS_MAC)
	FILE* truncateme = fopen("log.txt", "w");
	fclose(truncateme);
#endif
}

void ConsoleMessenger::outputMessage(Messenger::Severity severity, const char8* formatString, va_list argumentList) const
{
#if defined(__ANDROID__)
	// Note: There may be issues displaying 64bits values with this function
	// Note: This function will truncate long messages
	__android_log_vprint(messageTypes[severity], "com.powervr.Example", formatString, argumentList);
#elif defined(__QNXNTO__)
	vslogf(1, messageTypes[severity], formatString, argumentList);
#else //Not android Not QNX
	static char buffer[4096];
	va_list tempList;
	memset(buffer, 0, sizeof(buffer));
#if (defined _MSC_VER) // Pre VS2013
	tempList = argumentList;
#else
	va_copy(tempList, argumentList);
#endif
	vsnprintf(buffer, 4095, formatString, argumentList);

#if defined(_WIN32) && !defined(_CONSOLE)
	if (IsDebuggerPresent())
	{
		OutputDebugString(messageTypes[(int)severity]);
		OutputDebugString(buffer);
		OutputDebugString("\n");
	}
#else
	printf("%s", messageTypes[severity]);
	vprintf(formatString, tempList);
	printf("\n");
#endif
#if defined(PVR_PLATFORM_IS_DESKTOP) && !defined(TARGET_OS_MAC)
	{
		FILE* file = fopen("log.txt", "a");
		if (file)
		{
			fwrite(messageTypes[(int)severity], 1, strlen(messageTypes[(int)severity]), file);
			fwrite(buffer, 1, strlen(buffer), file);
			fwrite("\n", 1, 1, file);
			fclose(file);
		}
	}
#endif
#endif
}
}
}
//!\endcond