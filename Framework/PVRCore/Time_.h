/*!*********************************************************************************************************************
\file         PVRCore\Time_.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Class for dealing with time in a cross-platform way.
***********************************************************************************************************************/
#pragma once
#include "PVRCore/CoreIncludes.h"

// Required forward declarations
#if defined(__APPLE__)
struct mach_timebase_info;
#endif
namespace pvr {
/*!****************************************************************************************************
\brief         This class provides functions for measuring time: current time, elapsed time etc.
              High performance timers are used if available by the platform.
******************************************************************************************************/
class Time {
    public:
        Time();
        ~Time();
		/*!****************************************************************************************************
        \brief         Sets the current time as a the initial point to measure time from.
        ******************************************************************************************************/
        void Reset();
		/*!****************************************************************************************************
        \brief         Provides the time elapsed from the last call to Reset() or object construction.
        ******************************************************************************************************/
        uint64	getElapsedNanoSecs();
		/*!****************************************************************************************************
        \brief         Provides the time elapsed from the last call to Reset() or object construction.
        ******************************************************************************************************/
        uint64	getElapsedMicroSecs();
		/*!****************************************************************************************************
        \brief         Provides the time elapsed from the last call to Reset() or object construction.
        ******************************************************************************************************/
        uint64	getElapsedMilliSecs();
		/*!****************************************************************************************************
        \brief         Provides the time elapsed from the last call to Reset() or object construction.
        ******************************************************************************************************/
        uint64	getElapsedSecs();
		/*!****************************************************************************************************
        \brief         Provides the time elapsed from the last call to Reset() or object construction.
        ******************************************************************************************************/
        uint64	getElapsedMins();
		/*!****************************************************************************************************
        \brief         Provides the time elapsed from the last call to Reset() or object construction.
        ******************************************************************************************************/
        uint64	getElapsedHours();

		/*!****************************************************************************************************
        \brief         Provides the current time. Current time is abstract (not connected to the time of day) and
		              only useful for comparison.
        ******************************************************************************************************/
        uint64 getCurrentTimeNanoSecs() const;

		/*!****************************************************************************************************
        \brief         Provides the current time. Current time is abstract (not connected to the time of day) and
		              only useful for comparison.
        ******************************************************************************************************/
        uint64 getCurrentTimeMicroSecs() const;

		/*!****************************************************************************************************
        \brief         Provides the current time. Current time is abstract (not connected to the time of day) and
		              only useful for comparison.
        ******************************************************************************************************/
        uint64 getCurrentTimeMilliSecs() const;

		/*!****************************************************************************************************
        \brief         Provides the current time. Current time is abstract (not connected to the time of day) and
		              only useful for comparison.
        ******************************************************************************************************/
        uint64 getCurrentTimeSecs() const;

		/*!****************************************************************************************************
        \brief         Provides the current time. Current time is abstract (not connected to the time of day) and
		              only useful for comparison.
        ******************************************************************************************************/
        uint64 getCurrentTimeMins() const;

		/*!****************************************************************************************************
        \brief         Provides the current time. Current time is abstract (not connected to the time of day) and
		              only useful for comparison.
        ******************************************************************************************************/
        uint64 getCurrentTimeHours() const;
    private:
        uint64 getTimerFrequencyHertz() const;
        uint64 m_startTime;
        uint64 m_timerFrequency;

#ifdef _WIN32
        bool m_highResolutionSupported;
#elif defined(__APPLE__)
        struct mach_timebase_info* m_timeBaseInfo;
#endif
};
}