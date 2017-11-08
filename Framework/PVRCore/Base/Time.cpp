/*!
\brief Implementations of methods of the Time class.
\file PVRCore/Base/Time.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#elif defined(__APPLE__)
#include <mach/mach_time.h>

#elif defined(__QNX__)
#include <sys/time.h>

#else
#include <time.h>
#ifdef _POSIX_MONOTONIC_CLOCK
#define PVR_TIMER_CLOCK CLOCK_MONOTONIC
#else
#define PVR_TIMER_CLOCK  CLOCK_REALTIME
#endif
#endif

#include "PVRCore/Base/Time_.h"
#include <ctime>

namespace pvr {
Time::Time()
{
#if defined(_WIN32)
	_highResolutionSupported = QueryPerformanceFrequency((LARGE_INTEGER*)&_timerFrequency) != 0;
	if (!_highResolutionSupported)
	{
		// This is a rough estimate of GetTickCount64's frequency which is supposed to be sampled once between every 10-16ms, but there's no obvious way to query what the actual frequency is.
		// As this is a fall back method, it's probably not too much of an issue.
		_timerFrequency = 1000ull / 16ull;
	}
#elif defined(__APPLE__)
	_timeBaseInfo = new mach_timebase_info_data_t;
	mach_timebase_info(_timeBaseInfo);
	_timerFrequency = static_cast<uint64_t>((1.0e9 * (double)_timeBaseInfo->numer) / (double)_timeBaseInfo->denom);
#elif !defined(__QNX__)
	timespec timerInfo;
	if (clock_getres(PVR_TIMER_CLOCK, &timerInfo) != 0)
	{
		_timerFrequency = static_cast<uint64_t>(timerInfo.tv_sec);
	}
#endif

	//Reset the time so that we start afresh
	Reset();
}

Time::~Time()
{
#if defined(__APPLE__)
	delete _timeBaseInfo;
	_timeBaseInfo = 0;
#endif
}

void Time::Reset()
{
	_startTime = getCurrentTimeNanoSecs();
}

uint64_t	Time::getElapsedNanoSecs()
{
	return getCurrentTimeNanoSecs() - _startTime;
}

uint64_t Time::getElapsedMicroSecs()
{
	return getElapsedNanoSecs() / 1000ull;
}

uint64_t Time::getElapsedMilliSecs()
{
	return getElapsedNanoSecs() / 1000000ull;
}

uint64_t Time::getElapsedSecs()
{
	return getElapsedNanoSecs() / 1000000000ull;
}

uint64_t Time::getElapsedMins()
{
	return getElapsedNanoSecs() / (1000000000ull * 60ull);
}

uint64_t Time::getElapsedHours()
{
	return getElapsedNanoSecs() / (1000000000ull * 60ull * 60ull);
}

#if defined(_WIN32)
static inline uint64_t helperQueryPerformanceCounter()
{
	uint64_t lastTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&lastTime);
	return lastTime;
}
#endif

uint64_t Time::getCurrentTimeNanoSecs() const
{
	uint64_t currentTime;
#if defined(_WIN32)
	static uint64_t initialTime = helperQueryPerformanceCounter();
	if (_highResolutionSupported)
	{
		QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);
		currentTime = (1000000000 * (currentTime - initialTime)) / _timerFrequency;
	}
	else
	{
		currentTime = static_cast<uint64_t>(GetTickCount64() * 1000000ull);
	}

#elif defined(__APPLE__)
	uint64_t time = mach_absolute_time();
	currentTime = static_cast<uint64_t>(time * (_timeBaseInfo->numer / _timeBaseInfo->denom));
#elif defined(__QNX__)
	timeval tv;
	gettimeofday(&tv,NULL);
	currentTime = static_cast<uint64_t>((tv.tv_sec*(unsigned long)1000) + (tv.tv_usec/1000.0)) * 1000000;
#else
	timespec time;
	clock_gettime(PVR_TIMER_CLOCK, &time);
	currentTime = static_cast<uint64_t>(time.tv_nsec) +
	              // convert seconds to ns and add
	              1e9 * static_cast<uint64_t>(time.tv_sec);
#endif

	return currentTime;
}

uint64_t Time::getCurrentTimeMicroSecs() const
{
	return getCurrentTimeNanoSecs() / 1000ull;
}

uint64_t Time::getCurrentTimeMilliSecs() const
{
	return getCurrentTimeNanoSecs() / 1000000ull;
}

uint64_t Time::getCurrentTimeSecs() const
{
	//For seconds we can be slightly more accurate than just dividing the nano second amounts, so we re-implement the function properly.
	double retTime;
#if defined(_WIN32)
	if (_highResolutionSupported)
	{
		uint64_t currentTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);
		retTime = static_cast<double>(_timerFrequency);
	}
	else
	{
		retTime = (double)(GetTickCount64()) / 1000.;
	}

#elif defined(__APPLE__)
	uint64_t time = mach_absolute_time();
	retTime = static_cast<double>(time * _timerFrequency);
#elif defined(__QNX__)
	static uint64_t initialTime = clock();
	uint64_t currentTime = std::clock();
	retTime = static_cast<double>((currentTime - initialTime)) / CLOCKS_PER_SEC;
#else
	timespec time;
	clock_gettime(PVR_TIMER_CLOCK, &time);
	retTime = static_cast<double>(time.tv_sec);
#endif

	return static_cast<uint64_t>(retTime);
}

uint64_t Time::getCurrentTimeMins() const
{
	return getCurrentTimeSecs() / 60ull;
}

uint64_t Time::getCurrentTimeHours() const
{
	return getCurrentTimeSecs() / 3600ull;
}

uint64_t Time::getTimerFrequencyHertz() const
{
	return _timerFrequency;
}
}
//!\endcond