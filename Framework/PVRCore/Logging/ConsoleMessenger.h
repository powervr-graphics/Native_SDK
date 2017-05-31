/*!
\brief An implementation of the Messenger interface outputting to the Console window.
\file PVRCore/Logging/ConsoleMessenger.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/Logging/Messenger.h"

namespace pvr {
/// <summary>Contains functionality for the interaction of the Framework with the system it runs on (Console etc).
/// </summary>
namespace platform {
/// <summary>The ConsoleMessenger is an implementation of Messenger that outputs to the Console window.</summary>
/// <remarks>The ConsoleMessenger is an implementation of Messenger that outputs to the Console window.
/// Additionally, if a Debug window is detected (usually on Windows), the Messenger will try to use that.
/// Otherwise, if a Console is detected, it will output there. Lastly, in platforms where this is practical (i.e.
/// not sandboxed)it will output to a text file called log.txt.</remarks>
class ConsoleMessenger : public Messenger
{
private:
	/// <summary>Output a message if its severity is equal or greater than this messenger's severity threshold.
	/// </summary>
	/// <param name="severity">The message's severity</param>
	/// <param name="formatString">printf-style format string</param>
	/// <param name="argumentList">varargs list of printf-style variable arguments</param>
	void outputMessage(Messenger::Severity severity, const char8* formatString, va_list argumentList) const;
	void initializeMessenger();
};
}
}