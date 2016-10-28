/*!*********************************************************************************************************************
\file         PVRCore\Messenger.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains a class abstracting a messaging interface.
***********************************************************************************************************************/
#pragma once
#include "PVRCore/Defines.h"
#include <cstdarg>

namespace pvr {
namespace platform {
/*!*********************************************************************************************************************
\brief    Abstract class defining a messaging interface. This class implements the printing/messaging capabilities used by the
         Logger class. A class implementing it needs to implement the outputMessage function.
***********************************************************************************************************************/
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

	Messenger() : m_verbosityThreshold(
#ifdef DEBUG
		  Verbose
#else
		  Information
#endif

		) {}

	virtual ~Messenger() {};

	/*!*********************************************************************************************************************
	\brief    Abstract class defining a messaging interface. This class implements the printing/messaging capabilities used by the
	       Logger class. A class implementing it needs to implement the outputMessage function.
	***********************************************************************************************************************/
	virtual void output(Severity severity, const char8* formatString, va_list argumentList) const
	{
		if (severity >= getVerbosity())
		{
			outputMessage(severity, formatString, argumentList);
		}
	}

	/*!*********************************************************************************************************************
	\brief    Set the verbosity threshold below which messages will not be output.
	\param minimumLevelToOutput  The minimum level to actually output.
	\description  Messages with a severity less than this will be silently discarded. For example, if using a "Warning" level,
	         Critical, Error and Warning will be displayed, while Information, Verbose and Debug will be discarded.
	***********************************************************************************************************************/
	void setVerbosity(const Messenger::Severity minimumLevelToOutput) { m_verbosityThreshold = minimumLevelToOutput; }
	/*!*********************************************************************************************************************
	\brief    Get the verbosity threshold below which messages will not be output.
	\return The minimum level that is currently output.
	\description  Messages with a severity less than this will be silently discarded. For example, if using a "Warning" level,
	         Critical, Error and Warning will be displayed, while Information, Verbose and Debug will be discarded.
	***********************************************************************************************************************/
	Severity getVerbosity() const { return m_verbosityThreshold; }

protected:
	Severity m_verbosityThreshold;
private:
	virtual void outputMessage(Severity severity, const char8* formatString, va_list argumentList) const = 0;


};
}
}