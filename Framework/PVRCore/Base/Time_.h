/*!
\brief Class for dealing with time in a cross-platform way.
\file PVRCore/Base/Time_.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/CoreIncludes.h"

// Required forward declarations
#if defined(__APPLE__)
struct mach_timebase_info;
#endif
namespace pvr {
/// <summary>This class provides functions for measuring time: current time, elapsed time etc. High performance
/// timers are used if available by the platform.</summary>
class Time {
    public:
        Time();
        ~Time();
		/// <summary>Sets the current time as a the initial point to measure time from.</summary>
        void Reset();
		/// <summary>Provides the time elapsed from the last call to Reset() or object construction.</summary>
        uint64	getElapsedNanoSecs();
		/// <summary>Provides the time elapsed from the last call to Reset() or object construction.</summary>
        uint64	getElapsedMicroSecs();
		/// <summary>Provides the time elapsed from the last call to Reset() or object construction.</summary>
        uint64	getElapsedMilliSecs();
		/// <summary>Provides the time elapsed from the last call to Reset() or object construction.</summary>
        uint64	getElapsedSecs();
		/// <summary>Provides the time elapsed from the last call to Reset() or object construction.</summary>
        uint64	getElapsedMins();
		/// <summary>Provides the time elapsed from the last call to Reset() or object construction.</summary>
        uint64	getElapsedHours();

		/// <summary>Provides the current time. Current time is abstract (not connected to the time of day) and only useful for
		/// comparison.</summary>
        uint64 getCurrentTimeNanoSecs() const;

		/// <summary>Provides the current time. Current time is abstract (not connected to the time of day) and only useful for
		/// comparison.</summary>
        uint64 getCurrentTimeMicroSecs() const;

		/// <summary>Provides the current time. Current time is abstract (not connected to the time of day) and only useful for
		/// comparison.</summary>
        uint64 getCurrentTimeMilliSecs() const;

		/// <summary>Provides the current time. Current time is abstract (not connected to the time of day) and only useful for
		/// comparison.</summary>
        uint64 getCurrentTimeSecs() const;

		/// <summary>Provides the current time. Current time is abstract (not connected to the time of day) and only useful for
		/// comparison.</summary>
        uint64 getCurrentTimeMins() const;

		/// <summary>Provides the current time. Current time is abstract (not connected to the time of day) and only useful for
		/// comparison.</summary>
        uint64 getCurrentTimeHours() const;
    private:
        uint64 getTimerFrequencyHertz() const;
        uint64 _startTime;
        uint64 _timerFrequency;

#ifdef _WIN32
        bool _highResolutionSupported;
#elif defined(__APPLE__)
        struct mach_timebase_info* _timeBaseInfo;
#endif
};
}