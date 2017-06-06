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
	_timerFrequency = (uint64)((1.0e9 * (float64)_timeBaseInfo->numer) / (float64)_timeBaseInfo->denom);
#elif !defined(__QNX__)
	timespec timerInfo;
	if (clock_getres(PVR_TIMER_CLOCK, &timerInfo) != 0)
	{
		_timerFrequency = static_cast<uint64>(timerInfo.tv_sec);
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

uint64	Time::getElapsedNanoSecs()
{
	return getCurrentTimeNanoSecs() - _startTime;
}

uint64 Time::getElapsedMicroSecs()
{
	return getElapsedNanoSecs() / 1000ull;
}

uint64 Time::getElapsedMilliSecs()
{
	return getElapsedNanoSecs() / 1000000ull;
}

uint64 Time::getElapsedSecs()
{
	return getElapsedNanoSecs() / 1000000000ull;
}

uint64 Time::getElapsedMins()
{
	return getElapsedNanoSecs() / (1000000000ull * 60ull);
}

uint64 Time::getElapsedHours()
{
	return getElapsedNanoSecs() / (1000000000ull * 60ull * 60ull);
}

#if defined(_WIN32)
static inline uint64 helperQueryPerformanceCounter()
{
	uint64 lastTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&lastTime);
	return lastTime;
}
#endif

uint64 Time::getCurrentTimeNanoSecs() const
{
	uint64 currentTime;
#if defined(_WIN32)
	static uint64 initialTime = helperQueryPerformanceCounter();
	if (_highResolutionSupported)
	{
		QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);
		currentTime = (1000000000 * (currentTime - initialTime)) / _timerFrequency;
	}
	else
	{
		currentTime = static_cast<uint64>(GetTickCount64() * 1000000ull);
	}

#elif defined(__APPLE__)
	uint64_t time = mach_absolute_time();
	currentTime = static_cast<uint64>(time * (_timeBaseInfo->numer / _timeBaseInfo->denom));
#elif defined(__QNX__)
	timeval tv;
	gettimeofday(&tv,NULL);
	currentTime =(uint64)((tv.tv_sec*(unsigned long)1000) + (tv.tv_usec/1000.0)) * 1000000;
#else
	timespec time;
	clock_gettime(PVR_TIMER_CLOCK, &time);
	currentTime = static_cast<uint64>(time.tv_nsec) +
	              // convert seconds to ns and add
	              1e9 * static_cast<uint64>(time.tv_sec);
#endif

	return currentTime;
}

uint64 Time::getCurrentTimeMicroSecs() const
{
	return getCurrentTimeNanoSecs() / 1000ull;
}

uint64 Time::getCurrentTimeMilliSecs() const
{
	return getCurrentTimeNanoSecs() / 1000000ull;
}

uint64 Time::getCurrentTimeSecs() const
{
	//For seconds we can be slightly more accurate than just dividing the nano second amounts, so we re-implement the function properly.
	float64 retTime;
#if defined(_WIN32)
	if (_highResolutionSupported)
	{
		uint64 currentTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);
		retTime = static_cast<float64>(_timerFrequency);
	}
	else
	{
		retTime = (float64)(GetTickCount64()) / 1000.;
	}

#elif defined(__APPLE__)
	uint64_t time = mach_absolute_time();
	retTime = static_cast<float64>(time * _timerFrequency);
#elif defined(__QNX__)
	static uint64 initialTime = clock();
	uint64 currentTime = std::clock();
	retTime = static_cast<float64>((currentTime - initialTime)) / CLOCKS_PER_SEC;
#else
	timespec time;
	clock_gettime(PVR_TIMER_CLOCK, &time);
	retTime = static_cast<float64>(time.tv_sec);
#endif

	return static_cast<uint64>(retTime);
}

uint64 Time::getCurrentTimeMins() const
{
	return getCurrentTimeSecs() / 60ull;
}

uint64 Time::getCurrentTimeHours() const
{
	return getCurrentTimeSecs() / 3600ull;
}

uint64 Time::getTimerFrequencyHertz() const
{
	return _timerFrequency;
}
}
//!\endcond