/*!
\brief Implementation of logging functions
\file PVRSuperResolution/Log.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#include <stdexcept>
#include <string>
#include <assert.h>
#include <cstdlib>
#include <cstdarg>
#include <cstring>
#include <cstdio>

#include <assert.h>
#include <stdio.h>
#include "Log.h"

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#endif

#if defined(__linux__)
#include <fcntl.h>

#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#endif

#if defined(__ANDROID__)
#include <android/log.h>
static const android_LogPriority messageTypes[] = {
	ANDROID_LOG_VERBOSE,
	ANDROID_LOG_DEBUG,
	ANDROID_LOG_INFO,
	ANDROID_LOG_WARN,
	ANDROID_LOG_ERROR,
	ANDROID_LOG_FATAL,
};
#endif

namespace pvr {

bool isDebuggerPresent()
{
	// only check once for whether the debugger is present as this may not be efficient to determine
	static bool isUsingDebugger = false;
	static bool haveCheckedForDebugger = false;
	if (!haveCheckedForDebugger)
	{
#if defined(_MSC_VER)
		if (IsDebuggerPresent()) { isUsingDebugger = true; }
#elif defined(__APPLE__)
		// reference implementation taken from: https: // developer.apple.com/library/archive/qa/qa1361/_index.html
		int junk;
		int mib[4];
		struct kinfo_proc info;
		size_t size;

		// Initialize the flags so that, if sysctl fails for some bizarre
		// reason, we get a predictable result.

		info.kp_proc.p_flag = 0;

		// Initialize mib, which tells sysctl the info we want, in this case
		// we're looking for information about a specific process ID.

		mib[0] = CTL_KERN;
		mib[1] = KERN_PROC;
		mib[2] = KERN_PROC_PID;
		mib[3] = getpid();

		// Call sysctl.

		size = sizeof(info);
		junk = sysctl(mib, sizeof(mib) / sizeof(*mib), &info, &size, NULL, 0);
		assert(junk == 0);

		// We're being debugged if the P_TRACED flag is set.

		if ((info.kp_proc.p_flag & P_TRACED) != 0) { isUsingDebugger = true; }
#elif defined(__linux__)
		// reference implementation taken from: https://stackoverflow.com/a/24969863
		char buf[1024];

		int status_fd = open("/proc/self/status", O_RDONLY);
		if (status_fd == -1) { isUsingDebugger = false; }
		else
		{
			ssize_t num_read = read(status_fd, buf, sizeof(buf) - 1);
			if (num_read > 0)
			{
				static const char TracerPid[] = "TracerPid:";
				char* tracer_pid;

				buf[num_read] = 0;
				tracer_pid = strstr(buf, TracerPid);
				if (tracer_pid) { isUsingDebugger = !!atoi(tracer_pid + sizeof(TracerPid) - 1); }
			}
		}
#endif
		haveCheckedForDebugger = true;
	}

	return isUsingDebugger;
}

void logMessage(const char* formatString)
{
#if defined(__ANDROID__)
	__android_log_print(ANDROID_LOG_WARN, "com.imgtec.supernova", "%s", formatString);
#elif defined(__QNXNTO__)
	slogf(1, messageTypes[static_cast<uint32_t>(severity)], formatString);
#else // Not android Not QNX
	static char buffer[4096];
	memset(buffer, 0, sizeof(buffer));
	snprintf(buffer, 4095, "%s", formatString);
	buffer[4095] = 0;
#if defined(_WIN32)
	if (isDebuggerPresent())
	{
		OutputDebugString(buffer);
		OutputDebugString("\n");
	}
#endif
	printf("%s", formatString);
	printf("\n");
#endif
}

void assertCondition(bool condition, const char* msg)
{
	if (!condition)
	{
		logMessage(msg);
		assert(false);
	}
}

void assertFunctionResult(VkResult result, const char* msg)
{
	if (result != VkResult::VK_SUCCESS && result != VkResult::VK_SUBOPTIMAL_KHR)
	{
		logMessage(msg);
		assert(false);
	}
}

} // namespace pvr
