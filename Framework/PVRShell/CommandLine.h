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
		/// <summary>A c-style std::string name-value pair that represents command line argument (arg: name, val: value)
		/// </summary>
		struct Option
		{
			const char* arg; //!< Argument name (i.e. -Width)
			const char* val; //!< Argument value (i.e. 640)
			/// <summary>Equality</summary>
			/// <param name="rhs">Right hand side of the operator</param>
			/// <returns>True if left and right side are equal, otherwise false</returns>
			bool operator==(const Option& rhs) const
			{
				return strcmp(arg, rhs.arg) == 0;
			}
			/// <summary>Equality to c-string</summary>
			/// <param name="rhs">Right hand side of the operator</param>
			/// <returns>True if left and right side are equal, otherwise false</returns>
			bool operator==(const char* rhs) const
			{
				return strcmp(arg, rhs) == 0;
			}
		};
		typedef std::vector<Option> Options;//!< List of all options passed

		/// <summary>Get all command line options as a list of name/value pairs (c-strings)</summary>
		/// <returns>The command line options</returns>
		const Options& getOptionsList() const;

		/// <summary>Query if a specific argument name exists (regardless of the presence of a value or not). For
		/// example, if the command line was "myapp.exe -fps", the query hasOption("fps") will return true.</summary>
		/// <param name="name">The argument name to test</param>
		/// <returns>True if the argument name was passed through the command line , otherwise false.</returns>
		bool hasOption(const char* name) const;

		/// <summary>Get an argument as a std::string value. Returns false and leaves the value unchanged if the value is not
		/// present, allowing very easy use of default arguments.</summary>
		/// <param name="name">The command line argument (e.g. "-captureFrames")</param>
		/// <param name="outValue">The value passed with the argument (verbatim). If the name was not present, it remains
		/// unchanged</param>
		/// <returns>True if the argument "name" was present, false otherwise</returns>
		bool getStringOption(const char* name, std::string& outValue) const;
		/// <summary>Get an argument's value as a float value. Returns false and leaves the value unchanged if the value
		/// is not present, allowing very easy use of default arguments.</summary>
		/// <param name="name">The command line argument (e.g. "-captureFrames")</param>
		/// <param name="outValue">The value passed with the argument interpreted as a float. If the name was not present,
		/// it remains unchanged. If it was not representing a float, it silently returns zero (0.0).</param>
		/// <returns>True if the argument "name" was present, false otherwise</returns>
		bool getFloatOption(const char* name, float& outValue) const;

		/// <summary>Get an argument's value as an integer value. Returns false and leaves the value unchanged if the
		/// value is not present, allowing very easy use of default arguments.</summary>
		/// <param name="name">The command line argument (e.g. "-captureFrames")</param>
		/// <param name="outValue">The value passed with the argument interpreted as an integer. If the name was not
		/// present, it remains unchanged. If it was not representing a float, it silently returns zero (0).</param>
		/// <returns>True if the argument "name" was present, false otherwise</returns>
		bool getIntOption(const char* name, int32_t& outValue) const;

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
	/// <returns>The processed command line object.</returns>
	const ParsedCommandLine& getParsedCommandLine() const;


	/// <summary>Set the command line to a new std::string (wide).</summary>
	/// <param name="cmdLine">The new (wide) std::string to set the command line to</param>
	void set(const wchar_t* cmdLine);

	/// <summary>Set the command line to a new list of arguments.</summary>
	/// <param name="argc">The number of arguments</param>
	/// <param name="argv">The list of arguments</param>
	void set(int argc, char** argv);

	/// <summary>Set the command line from a new std::string.</summary>
	/// <param name="cmdLine">The new std::string to set the command line to</param>
	void set(const char* cmdLine);

	/// <summary>Set the command line from a stream.</summary>
	/// <param name="stream">The stream containing the new command line</param>
	void set(Stream* stream);

	/// <summary>Set the command line from another command line.</summary>
	/// <param name="commandLine">Copy the data from another command line</param>
	void set(const CommandLineParser& commandLine);

	/// <summary>Prepend data to the command line.</summary>
	/// <param name="cmdLine">A std::string containing the data to prepend to this command line</param>
	void prefix(const wchar_t* cmdLine);

	/// <summary>Prepend a new list of arguments to the command line.</summary>
	/// <param name="argc">The number of arguments</param>
	/// <param name="argv">The list of arguments</param>
	void prefix(int argc, char** argv);

	/// <summary>Prepend data from a std::string to the command line.</summary>
	/// <param name="cmdLine">The std::string whose data to prepend to the command line</param>
	void prefix(const char* cmdLine);

	/// <summary>Prepend data from a stream to the command line.</summary>
	/// <param name="cmdLine">The stream whose data to prepend to the command line</param>
	void prefix(Stream* const cmdLine);

	/// <summary>Prepend the data from another command line.</summary>
	/// <param name="cmdLine">The command line from which to prepend the data</param>
	void prefix(const CommandLineParser& cmdLine);

	/// <summary>Append data to the command line.</summary>
	/// <param name="cmdLine">A std::string containing the data to append to this command line</param>
	void append(const wchar_t* cmdLine);

	/// <summary>Append a new list of arguments to the command line.</summary>
	/// <param name="argc">The number of arguments</param>
	/// <param name="argv">The list of arguments</param>
	void append(int argc, char** argv);

	/// <summary>Append data from a std::string to the command line.</summary>
	/// <param name="cmdLine">The std::string whose data to append to the command line</param>
	void append(const char* cmdLine);

	/// <summary>Append data from a stream to the command line.</summary>
	/// <param name="cmdLine">The stream whose data to append to the command line</param>
	void append(Stream* cmdLine);

	/// <summary>Append data from from another command line.</summary>
	/// <param name="cmdLine">The command line from which to append the data</param>
	void append(const CommandLineParser& cmdLine);

protected:
	/// <summary>Parse an entire std::string for command line data.</summary>
	/// <param name="cmdLine">The std::string containing the command line data</param>
	void parseCmdLine(const char* const cmdLine);

	/// <summary>Parse a single argument as passed by the C/C++ style argc/argv command line format.</summary>
	/// <param name="arg">A single element of the "argv" array of char*</param>
	void parseArgV(char* arg);

private:
	uint32_t findArg(const char* pArg) const;
	bool readFlag(const char* pArg, bool& bVal) const;
	bool readUint(const char* pArg, unsigned int& uiVal) const;
	bool readFloat(const char* pArg, float& fVal) const;
	CharBuffer _data;
	ParsedCommandLine _commandLine;

};
}
///<summary> Typedef of the CommandLine into the pvr namespace</summary>
typedef platform::CommandLineParser::ParsedCommandLine CommandLine;
}