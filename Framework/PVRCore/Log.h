/*!
\brief This file contains Logging functionality. Look here if plan to use custom logging. Default logging accessed
through the global Log functor (Log("a message")).
\file PVRCore/Log.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/Logging/ConsoleMessenger.h"
namespace pvr {

/// <summary>Represents an object capable of providing Logging functionality. This class is normally instantiated and
/// configured, not inherited from. The components providing the Logging capability are contained in this class
/// through interfaces, and as such can be replaced with custom components.</summary>
class Logger
{
private:
	static platform::ConsoleMessenger _defaultMessageHandler;
	platform::Messenger* _messageHandler;
public:
	/// <summary>Enumerates possible severities from Critical down to Debug.</summary>
	enum Severity
	{
		None = platform::Messenger::None,
		Debug = platform::Messenger::Debug,
		Verbose = platform::Messenger::Verbose,
		Information = platform::Messenger::Information,
		Warning = platform::Messenger::Warning,
		Error = platform::Messenger::Error,
		Critical = platform::Messenger::Critical,
	};

	/// <summary>Default constructor. Will construct a logger using the Default message handler. The default message
	/// handler uses suitable system-specific output: Console output for console systems and debug environments,
	/// system logging for mobile systems, file output for consoleless desktop.</summary>
	Logger() : _messageHandler(&_defaultMessageHandler) {}

	/// <summary>Functor operator used to allow an instance of this class to be called as a function. Logs a message using
	/// this logger's message handler.</summary>
	/// <param name="severity">The severity of the message. Apart from being output into the message, the severity is
	/// used by the logger to discard log events less than a specified threshold. See setVerbosity(...)</param>
	/// <param name="formatString">A printf-style format string</param>
	/// <param name="...">Variable arguments for the format string. Printf-style rules</param>
	void operator()(Severity severity, const char8* const formatString, ...)
	{
		va_list argumentList;
		va_start(argumentList, formatString);
		vaOutput(severity, formatString, argumentList);
		va_end(argumentList);
	}

	/// <summary>Functor operator used to allow an instance of this class to be called as a function. Logs a message using
	/// this logger's message handler. Severity is fixed to "Error".</summary>
	/// <param name="formatString">A printf-style format string</param>
	/// <param name="...">Variable arguments for the format string. Printf-style rules</param>
	void operator()(const char8* const formatString, ...)
	{
		va_list argumentList;
		va_start(argumentList, formatString);
		vaOutput(Error, formatString, argumentList);
		va_end(argumentList);
	}

	/// <summary>Logs a message using this logger's message handler.</summary>
	/// <param name="severity">The severity of the message. Apart from being output into the message, the severity is
	/// used by the logger to discard log events less than a specified threshold. See setVerbosity(...)</param>
	/// <param name="formatString">A printf-style format string</param>
	/// <param name="...">Variable arguments for the format string. Printf-style rules</param>
	void output(Severity severity, const char8* const formatString, ...)
	{
		va_list argumentList;
		va_start(argumentList, formatString);
		vaOutput(severity, formatString, argumentList);
		va_end(argumentList);
	}

	/// <summary>Logs a message using the Default message handler.</summary>
	/// <param name="severity">The severity of the message.</param>
	/// <param name="formatString">A printf-style format string</param>
	/// <param name="...">Variable arguments for the format string. Printf-style rules</param>
	static void static_output(Severity severity, const char8* const formatString, ...)
	{
		va_list argumentList;
		va_start(argumentList, formatString);
		static_vaOutput(severity, formatString, argumentList);
		va_end(argumentList);
	}

	/// <summary>Varargs version of the "output" function.</summary>
	void vaOutput(Severity severity, const char8* const formatString, va_list argumentList)
	{
		if (_messageHandler)
		{
			_messageHandler->output(static_cast<platform::Messenger::Severity>(severity), formatString, argumentList);
		}
	}

	/// <summary>Varargs version of the "static_output" function.</summary>
	static void static_vaOutput(Severity severity, const char8* const formatString, va_list argumentList)
	{
		_defaultMessageHandler.output(static_cast<platform::Messenger::Severity>(severity), formatString, argumentList);
	}

	/// <summary>Get the Messenger object that acts as this Logger's message handler.</summary>
	/// <returns>The Messenger object that acts as this Logger's message handler.</returns>
	const platform::Messenger* getMessageHandler()
	{
		return _messageHandler;
	}

	/// <summary>Sets the Messenger object that acts as this Logger's message handler.</summary>
	/// <param name="messageHandler">The Messenger object that is to act as this Logger's message handler.</param>
	void setMessageHandler(platform::Messenger* messageHandler)
	{
		Logger::_messageHandler = messageHandler;
	}

	/// <summary>Get the Verbosity threshold of this Logger.</summary>
	/// <returns>The Verbosity threshold of this Logger.</returns>
	Severity getVerbosity()
	{
		return static_cast<Logger::Severity>(_messageHandler->getVerbosity());
	}

	/// <summary>Sets the Verbosity threshold of this Logger.</summary>
	/// <param name="verbosity">The Verbosity threshold that is to be set to this Logger.</param>
	void setVerbosity(const Severity verbosity)
	{
		_messageHandler->setVerbosity((platform::Messenger::Severity)verbosity);
	}

	/// <summary>Use this function to convert a Result into a string that is suitable for outputting.</summary>
	/// <returns>A string suitable for writing out that represents this Result</returns>
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
	void initializeMessenger() { _messageHandler->initialize(); }
};

/// <summary>--- GLOBAL OBJECT THAT IS NORMALLY THE DEFAULT LOG --- Use to log your issues.</summary>
/// <remarks>Normally used as a functor: Log(Log.Warning, "This is warning number %d", 42)</remarks>
extern ::pvr::Logger Log;

/// <summary>Callback used for some classes to allow them to log errors. static_output follows this signature.
/// </summary>
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