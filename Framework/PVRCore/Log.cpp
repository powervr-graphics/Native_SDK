/*!*********************************************************************************************************************
\file         PVRCore\Log.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief			Implementation for the Logger class and the global Log object.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include <cstdarg>
#include <cstddef>
#include <string>
#include "PVRCore/Log.h"


namespace pvr {
//Set the default message handler creation to be the basic CPVRMessenger class.
platform::ConsoleMessenger Logger::m_defaultMessageHandler = platform::ConsoleMessenger();

Logger Log;

}
//!\endcond