/*!
\brief Implementation for the Logger class and the global Log object.
\file PVRCore/Logging/Log.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include <cstdarg>
#include <cstddef>
#include <string>
#include "PVRCore/Log.h"


namespace pvr {
//Set the default message handler creation to be the basic CPVRMessenger class.
platform::ConsoleMessenger Logger::_defaultMessageHandler = platform::ConsoleMessenger();

Logger Log;

}
//!\endcond