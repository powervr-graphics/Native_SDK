/*!*********************************************************************************************************************
\file         PVRCore\Log.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         This file contains Logging functionality. Look here if plan to use custom logging. Default logging accessed through
              the global Log functor (Log("a message")).
***********************************************************************************************************************/
#pragma once
#include "PVRCore/ConsoleMessenger.h"
namespace pvr {

/*!****************************************************************************************************************
\brief     Represents an object capable of providing Logging functionality. This class is normally instantiated
           and configured, not inherited from. The components providing the Logging capability are contained in
		   this class through interfaces, and as such can be replaced with custom components.
*******************************************************************************************************************/
class Logger
{
private:
	static platform::ConsoleMessenger m_defaultMessageHandler;
	platform::Messenger* m_messageHandler;

public:
	/*!****************************************************************************************************************
	\brief     Enumerates possible severities from Critical down to Debug.
	*******************************************************************************************************************/
	enum Severity
	{
		Debug = platform::Messenger::Debug,
		Verbose = platform::Messenger::Verbose,
		Information = platform::Messenger::Information,
		Warning = platform::Messenger::Warning,
		Error = platform::Messenger::Error,
		Critical = platform::Messenger::Critical,
	};

	/*!****************************************************************************************************************
	\brief     Default constructor. Will construct a logger using the Default message handler. The default message
	           handler uses suitable system-specific output: Console output for console systems and debug environments,
			   system logging for mobile systems, file output for consoleless desktop.
	*******************************************************************************************************************/
	Logger() : m_messageHandler(&m_defaultMessageHandler) {}

	/*!****************************************************************************************************************
	\brief     Functor operator used to allow an instance of this class to be called as a function. Logs a message using
	           this logger's message handler.
	\param     severity The severity of the message. Apart from being output into the message, the severity is used by
	           the logger to discard log events less than a specified threshold. See setVerbosity(...)
	\param     formatString  A printf-style format string
	\param     ...  Variable arguments for the format string. Printf-style rules
	*******************************************************************************************************************/
	void operator()(Severity severity, const char8* const formatString, ...)
	{
		va_list argumentList;
		va_start(argumentList, formatString);
		vaOutput(severity, formatString, argumentList);
		va_end(argumentList);
	}

	/*!****************************************************************************************************************
	\brief     Functor operator used to allow an instance of this class to be called as a function. Logs a message using
	           this logger's message handler. Severity is fixed to "Error".
	\param     formatString  A printf-style format string
	\param     ...  Variable arguments for the format string. Printf-style rules
	*******************************************************************************************************************/
	void operator()(const char8* const formatString, ...)
	{
		va_list argumentList;
		va_start(argumentList, formatString);
		vaOutput(Error, formatString, argumentList);
		va_end(argumentList);
	}

	/*!****************************************************************************************************************
	\brief     Logs a message using this logger's message handler.
	\param     severity The severity of the message. Apart from being output into the message, the severity is used by
	           the logger to discard log events less than a specified threshold. See setVerbosity(...)
	\param     formatString  A printf-style format string
	\param     ...  Variable arguments for the format string. Printf-style rules
	*******************************************************************************************************************/
	void output(Severity severity, const char8* const formatString, ...)
	{
		va_list argumentList;
		va_start(argumentList, formatString);
		vaOutput(severity, formatString, argumentList);
		va_end(argumentList);
	}

	/*!****************************************************************************************************************
	\brief     Logs a message using the Default message handler.
	\param     severity The severity of the message.
	\param     formatString  A printf-style format string
	\param     ...  Variable arguments for the format string. Printf-style rules
	*******************************************************************************************************************/
	static void static_output(Severity severity, const char8* const formatString, ...)
	{
		va_list argumentList;
		va_start(argumentList, formatString);
		static_vaOutput(severity, formatString, argumentList);
		va_end(argumentList);
	}

	/*!****************************************************************************************************************
	\brief     Varargs version of the "output" function.
	*******************************************************************************************************************/
	void vaOutput(Severity severity, const char8* const formatString, va_list argumentList)
	{
		if (m_messageHandler)
		{
			m_messageHandler->output(static_cast<platform::Messenger::Severity>(severity), formatString, argumentList);
		}
	}

	/*!****************************************************************************************************************
	\brief     Varargs version of the "static_output" function.
	*******************************************************************************************************************/
	static void static_vaOutput(Severity severity, const char8* const formatString, va_list argumentList)
	{
		m_defaultMessageHandler.output(static_cast<platform::Messenger::Severity>(severity), formatString, argumentList);
	}

	/*!****************************************************************************************************************
	\return     The Messenger object that acts as this Logger's message handler.
	*******************************************************************************************************************/
	const platform::Messenger* getMessageHandler()
	{
		return m_messageHandler;
	}

	/*!****************************************************************************************************************
	\brief     Sets the Messenger object that acts as this Logger's message handler.
	\param     messageHandler The Messenger object that is to act as this Logger's message handler.
	*******************************************************************************************************************/
	void setMessageHandler(platform::Messenger* messageHandler)
	{
		Logger::m_messageHandler = messageHandler;
	}

	/*!****************************************************************************************************************
	\return     The Verbosity threshold of this Logger.
	*******************************************************************************************************************/
	Severity getVerbosity()
	{
		return static_cast<Logger::Severity>(m_messageHandler->getVerbosity());
	}

	/*!****************************************************************************************************************
	\brief     Sets the Verbosity threshold of this Logger.
	\param     verbosity The Verbosity threshold that is to be set to this Logger.
	*******************************************************************************************************************/
	void setVerbosity(const Severity verbosity)
	{
		m_messageHandler->setVerbosity((platform::Messenger::Severity)verbosity);
	}

	/*!****************************************************************************************************************
	\brief      Use this function to convert a Result into a string that is suitable for outputting.
	\return     A string suitable for writing out that represents this Result
	*******************************************************************************************************************/
	static const char* getResultCodeString(Result result)
	{
		switch (result)
		{
		case Result::Success: return "Success";
		case Result::UnknownError: return"Unknown Error";
		case Result::OutOfMemory: return"Out Of Memory";
		case Result::InvalidArgument: return"Invalid Argument";
		case Result::AlreadyInitialized: return"Already Initialized";
		case Result::NotInitialized: return"Not Initialized";
		case Result::UnsupportedRequest: return"Unsupported Request";
		case Result::FileVersionMismatch: return"File Version Mismatch";
		case Result::NotReadable: return"Not Readable";
		case Result::NotWritable: return"Not Writable";
		case Result::EndOfStream: return"End Of Stream";
		case Result::UnableToOpen: return"Unable To Open";
		case Result::NoData: return"No Data";
		case Result::OutOfBounds: return"Out Of Bounds";
		case Result::NotFound: return"Not Found";
		case Result::KeyAlreadyExists: return"Key Already Exists";
		case Result::ExitRenderFrame: return"Exit Render Scene";
		case Result::InvalidData: return"Invalid Data";
		default: return"UNRECOGNIZED CODE";
		}
	}

};

/*!****************************************************************************************************************
\brief         --- GLOBAL OBJECT THAT IS NORMALLY THE DEFAULT LOG --- Use to log your issues.
\description   Normally used as a functor:     Log(Log.Warning, "This is warning number %d", 42)
*******************************************************************************************************************/
extern ::pvr::Logger Log;

/*!****************************************************************************************************************
\brief         Callback used for some classes to allow them to log errors. static_output follows this signature.
*******************************************************************************************************************/
typedef void(*ErrorLogger)(Logger::Severity severity, const char8*, ...);



inline void assertion(bool condition, const std::string& message)
{
	if (!condition)
	{
		Log(Log.Critical, ("ASSERTION FAILED: " + message).c_str());
		PVR_ASSERTION(0);
	}
}

#define SLASH(s) /##s
#define COMMENT SLASH(/)

#ifdef DEBUG
#define debug_assertion(condition, message) assertion(condition, message)
#else
#define debug_assertion(condition, message) ((void)0)
#endif
inline void assertion(bool condition, const char* msg)
{
	if (!condition)
	{
		assertion(condition, std::string(msg));
	}
}

inline void assertion(bool condition)
{
	assertion(condition, "");
}
}