/*!*********************************************************************************************************************
\file         PVRCore\ConsoleMessenger.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         An implementation of the Messenger interface outputting to the Console window.
***********************************************************************************************************************/
#pragma once
#include "PVRCore/Messenger.h"

namespace pvr {
/*!*********************************************************************************************************************
\brief       Contains functionality for the interaction of the Framework with the system it runs on (Console etc).
***********************************************************************************************************************/
namespace platform {
/*!*********************************************************************************************************************
\brief       The ConsoleMessenger is an implementation of Messenger that outputs to the Console window.
\description  The ConsoleMessenger is an implementation of Messenger that outputs to the Console window.
				Additionally, if a Debug window is detected (usually on Windows), the Messenger will try to use that. Otherwise,
             if a Console is detected, it will output there.
			 Lastly, in platforms where this is practical (i.e. not sandboxed)it will output to a text file called log.txt.
***********************************************************************************************************************/
class ConsoleMessenger : public Messenger
{
public:
	ConsoleMessenger();
	~ConsoleMessenger();

private:
	/*!*********************************************************************************************************************
	\brief       Output a message if its severity is equal or greater than this messenger's severity threshold.
	\param       severity The message's severity
	\param       formatString printf-style format string
	\param       argumentList varargs list of printf-style variable arguments
	***********************************************************************************************************************/
	void outputMessage(Messenger::Severity severity, const char8* formatString, va_list argumentList) const;
};
}
}