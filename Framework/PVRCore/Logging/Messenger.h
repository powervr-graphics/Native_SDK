/*!
\brief Contains a class abstracting a messaging interface.
\file PVRCore/Logging/Messenger.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/Base/Defines.h"
#include <cstdarg>

namespace pvr {
namespace platform {
/// <summary>Abstract class defining a messaging interface. This class implements the printing/messaging capabilities
/// used by the Logger class. A class implementing it needs to implement the outputMessage function.</summary>
class Messenger
{
public:
	enum  Severity
	{
		Verbose = 0,
		Debug = 1,
		Information = 2,
		Warning = 3,
		Error = 4,
		Critical = 5,
		None = 6,
	};

	Messenger() : _verbosityThreshold(
#ifdef DEBUG
		  Verbose
#else
		  Information
#endif

		) {}

	virtual ~Messenger() {};

	/// <summary>Abstract class defining a messaging interface. This class implements the printing/messaging capabilities
	/// used by the Logger class. A class implementing it needs to implement the outputMessage function.</summary>
	virtual void output(Severity severity, const char8* formatString, va_list argumentList) const
	{
		if (severity >= getVerbosity())
		{
			outputMessage(severity, formatString, argumentList);
		}
	}

	/// <summary>Set the verbosity threshold below which messages will not be output.</summary>
	/// <param name="minimumLevelToOutput">The minimum level to actually output.</param>
	/// <remarks>Messages with a severity less than this will be silently discarded. For example, if using a "Warning"
	/// level, Critical, Error and Warning will be displayed, while Information, Verbose and Debug will be discarded.
	/// </remarks>
	void setVerbosity(const Messenger::Severity minimumLevelToOutput) { _verbosityThreshold = minimumLevelToOutput; }
	/// <summary>Get the verbosity threshold below which messages will not be output.</summary>
	/// <returns>The minimum level that is currently output.</returns>
	/// <remarks>Messages with a severity less than this will be silently discarded. For example, if using a "Warning"
	/// level, Critical, Error and Warning will be displayed, while Information, Verbose and Debug will be discarded.
	/// </remarks>
	Severity getVerbosity() const { return _verbosityThreshold; }

	void initialize() { initializeMessenger(); }
protected:
	Severity _verbosityThreshold;
private:
	virtual void outputMessage(Severity severity, const char8* formatString, va_list argumentList) const = 0;
	virtual void initializeMessenger() = 0;


};
}
}