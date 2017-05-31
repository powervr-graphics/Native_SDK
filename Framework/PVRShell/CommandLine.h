/*!
\brief Contains the CommandLine class.
\file PVRShell/CommandLine.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRShell/ShellIncludes.h"

namespace pvr {
namespace platform {

/// <summary>This class parses, abstracts, stores and handles command line options passed on application launch.
/// </summary>
class CommandLineParser
{
public:
	/// <summary>This class provides access to the command line arguments of a CommandLineParser. Its lifecycle is tied
	/// to the commandLineParser.</summary>
	class ParsedCommandLine
	{
	public:
		/// <summary>A c-style string name-value pair that represents command line argument (arg: name, val: value)
		/// </summary>
		struct Option
		{
			const char* arg; //!< Argument name (i.e. -Width)
			const char* val; //!< Argument value (i.e. 640)
			bool operator==(const Option& rhs) const
			{
				return strcmp(arg, rhs.arg) == 0;
			}
			bool operator==(const char* rhs) const
			{
				return strcmp(arg, rhs) == 0;
			}
		};
		typedef std::vector<Option> Options;//!< List of all options passed

		/// <summary>Get all command line options as a list of c-string name/value pairs</summary>
		const Options& getOptionsList() const;

		/// <summary>Query if a specific argument name exists</summary>
		bool hasOption(const char* name) const;

		/// <summary>Get an argument as a string value. Returns false and leaves the value unchanged if the value is not
		/// present, allowing very easy use of default arguments.</summary>
		/// <param name="name">The command line argument (e.g. "-captureFrames")</param>
		/// <param name="outValue">The value passed with the argument (verbatim). If the name was not present, it remains
		/// unchanged</param>
		/// <returns>True if the argument "name" was present, false otherwise</returns>
		bool getStringOption(const char* name, std::string& outValue) const;
<<<<<<< HEAD
		/*!*********************************************************************************************************************
		\brief  Get an argument's value as a float value. Returns false and leaves the value unchanged if the value is not present,
		allowing very easy use of default arguments.
		\param[in]	name The command line argument (e.g. "-captureFrames")
		\param[out]	outValue The value passed with the argument interpreted as a float. If the name was not present, it remains
		unchanged. If it was not representing a float, it silently returns zero (0.0).
		\return	True if the argument "name" was present, false otherwise
		***********************************************************************************************************************/
		bool getFloatOption(const char* name, float32& outValue) const;

		/*!*********************************************************************************************************************
		\brief  Get an argument's value as an integer value. Returns false and leaves the value unchanged if the value is not present,
		allowing very easy use of default arguments.
		\param[in]	name The command line argument (e.g. "-captureFrames")
		\param[out]	outValue The value passed with the argument interpreted as an integer. If the name was not present, it remains
		unchanged. If it was not representing a float, it silently returns zero (0).
		\return	True if the argument "name" was present, false otherwise
		***********************************************************************************************************************/
=======
		/// <summary>Get an argument's value as a float value. Returns false and leaves the value unchanged if the value
		/// is not present, allowing very easy use of default arguments.</summary>
		/// <param name="name">The command line argument (e.g. "-captureFrames")</param>
		/// <param name="outValue">The value passed with the argument interpreted as a float. If the name was not present,
		/// it remains unchanged. If it was not representing a float, it silently returns zero (0.0).</param>
		/// <returns>True if the argument "name" was present, false otherwise</returns>
		bool getFloatOption(const char* name, float32& outValue) const;

		/// <summary>Get an argument's value as an integer value. Returns false and leaves the value unchanged if the
		/// value is not present, allowing very easy use of default arguments.</summary>
		/// <param name="name">The command line argument (e.g. "-captureFrames")</param>
		/// <param name="outValue">The value passed with the argument interpreted as an integer. If the name was not
		/// present, it remains unchanged. If it was not representing a float, it silently returns zero (0).</param>
		/// <returns>True if the argument "name" was present, false otherwise</returns>
>>>>>>> 1776432f... 4.3
		bool getIntOption(const char* name, int32& outValue) const;

		/// <summary>If a specific argument was present, set outValue to True.</summary>
		/// <param name="name">The command line argument (e.g. "-captureFrames")</param>
		/// <param name="outValue">True if the argument "name" was present, otherwise unchanged</param>
		/// <returns>True if the argument "name" was present, false otherwise</returns>
		bool getBoolOptionSetTrueIfPresent(const char* name, bool& outValue) const;

		/// <summary>If a specific argument was present, set outValue to False.</summary>
		/// <param name="name">The command line argument (e.g. "-captureFrames")</param>
		/// <param name="outValue">False if the argument "name" was present, otherwise unchanged</param>
		/// <returns>True if the argument "name" was present, false otherwise</returns>
		bool getBoolOptionSetFalseIfPresent(const char* name, bool& outValue) const;

	private:
		friend class CommandLineParser;
		Options _options;
	};

	/// <summary>Constructor.</summary>
	CommandLineParser();

	/// <summary>Get a ParsedCommandLine option to inspect and use the command line arguments.</summary>
	const ParsedCommandLine& getParsedCommandLine() const;


	/// <summary>Set the command line to a new string (wide).</summary>
	/// <param name="cmdLine">The new (wide) string to set the command line to</param>
	void set(const wchar_t* cmdLine);

	/// <summary>Set the command line to a new list of arguments.</summary>
	/// <param name="argc">The number of arguments</param>
	/// <param name="argv">The list of arguments</param>
	void set(int argc, char** argv);

	/// <summary>Set the command line from a new string.</summary>
	/// <param name="cmdLine">The new string to set the command line to</param>
	void set(const char* cmdLine);

	/// <summary>Set the command line from a stream.</summary>
	/// <param name="stream">The stream containing the new command line</param>
	void set(Stream* stream);

	/// <summary>Set the command line from another command line.</summary>
	/// <param name="commandLine">Copy the data from another command line</param>
	void set(const CommandLineParser& commandLine);

	/// <summary>Prepend data to the command line.</summary>
	/// <param name="cmdLine">A string containing the data to prepend to this command line</param>
	void prefix(const wchar_t* cmdLine);

	/// <summary>Prepend a new list of arguments to the command line.</summary>
	/// <param name="argc">The number of arguments</param>
	/// <param name="argv">The list of arguments</param>
	void prefix(int argc, char** argv);

	/// <summary>Prepend data from a string to the command line.</summary>
	/// <param name="cmdLine">The string whose data to prepend to the command line</param>
	void prefix(const char* cmdLine);

	/// <summary>Prepend data from a stream to the command line.</summary>
	/// <param name="cmdLine">The stream whose data to prepend to the command line</param>
	void prefix(Stream* const cmdLine);

	/// <summary>Prepend the data from another command line.</summary>
	/// <param name="cmdLine">The command line from which to prepend the data</param>
	void prefix(const CommandLineParser& cmdLine);

	/// <summary>Append data to the command line.</summary>
	/// <param name="cmdLine">A string containing the data to append to this command line</param>
	void append(const wchar_t* cmdLine);

	/// <summary>Append a new list of arguments to the command line.</summary>
	/// <param name="argc">The number of arguments</param>
	/// <param name="argv">The list of arguments</param>
	void append(int argc, char** argv);

	/// <summary>Append data from a string to the command line.</summary>
	/// <param name="cmdLine">The string whose data to append to the command line</param>
	void append(const char* cmdLine);

	/// <summary>Append data from a stream to the command line.</summary>
	/// <param name="cmdLine">The stream whose data to append to the command line</param>
	void append(Stream* cmdLine);

	/// <summary>Append data from from another command line.</summary>
	/// <param name="cmdLine">The command line from which to append the data</param>
	void append(const CommandLineParser& cmdLine);

protected:
	/// <summary>Parse an entire string for command line data.</summary>
	/// <param name="cmdLine">The string containing the command line data</param>
	void parseCmdLine(const char8* const cmdLine);

	/// <summary>Parse a single argument as passed by the C/C++ style argc/argv command line format.</summary>
	/// <param name="arg">A single element of the "argv" array of char*</param>
	void parseArgV(char* arg);

private:
	uint32 findArg(const char* pArg) const;
	bool readFlag(const char* pArg, bool& bVal) const;
	bool readUint(const char* pArg, unsigned int& uiVal) const;
	bool readFloat(const char* pArg, float& fVal) const;
	CharBuffer _data;
	ParsedCommandLine _commandLine;

};
typedef CommandLineParser::ParsedCommandLine CommandLine;
}

}