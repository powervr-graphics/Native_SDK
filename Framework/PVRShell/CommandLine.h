/*!*********************************************************************************************************************
\file         PVRShell\CommandLine.h
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Contains the CommandLine class.
***********************************************************************************************************************/
#pragma once
#include "PVRShell/ShellIncludes.h"

namespace pvr {
namespace platform {

/*!*********************************************************************************************************************
\brief        This class parses, abstracts, stores and handles command line options passed on application launch.
***********************************************************************************************************************/
class CommandLineParser
{
public:
	/*!*********************************************************************************************************************
	\brief        This class provides access to the command line arguments of a CommandLineParser. Its lifecycle is tied
	to the commandLineParser.
	***********************************************************************************************************************/
	class ParsedCommandLine
	{
	public:
		/*!*********************************************************************************************************************
		\brief        A c-style string name-value pair that represents command line argument (arg: name, val: value)
		***********************************************************************************************************************/
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

		/*!*********************************************************************************************************************
		\brief        Get all command line options as a list of c-string name/value pairs
		***********************************************************************************************************************/
		const Options& getOptionsList() const;

		/*!*********************************************************************************************************************
		\brief        Query if a specific argument name exists
		***********************************************************************************************************************/
		bool hasOption(const char* name) const;

		/*!*********************************************************************************************************************
		\brief  Get an argument as a string value. Returns false and leaves the value unchanged if the value is not present,
				allowing very easy use of default arguments.
		\param[in]	name The command line argument (e.g. "-captureFrames")
		\param[out]	outValue The value passed with the argument (verbatim). If the name was not present, it remains unchanged
		\return	True if the argument "name" was present, false otherwise
		***********************************************************************************************************************/
		bool getStringOption(const char* name, std::string& outValue) const;
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
		bool getIntOption(const char* name, int32& outValue) const;

		/*!*********************************************************************************************************************
		\brief  If a specific argument was present, set outValue to True.
		\param[in]	name The command line argument (e.g. "-captureFrames")
		\param[out]	outValue True if the argument "name" was present, otherwise unchanged
		\return	True if the argument "name" was present, false otherwise
		***********************************************************************************************************************/
		bool getBoolOptionSetTrueIfPresent(const char* name, bool& outValue) const;

		/*!*********************************************************************************************************************
		\brief  If a specific argument was present, set outValue to False.
		\param[in]	name The command line argument (e.g. "-captureFrames")
		\param[out]	outValue False if the argument "name" was present, otherwise unchanged
		\return	True if the argument "name" was present, false otherwise
		***********************************************************************************************************************/
		bool getBoolOptionSetFalseIfPresent(const char* name, bool& outValue) const;

	private:
		friend class CommandLineParser;
		Options m_options;
	};

	/*!*********************************************************************************************************************
	\brief        Constructor.
	***********************************************************************************************************************/
	CommandLineParser();

	/*!*********************************************************************************************************************
	\brief        Get a ParsedCommandLine option to inspect and use the command line arguments.
	***********************************************************************************************************************/
	const ParsedCommandLine& getParsedCommandLine() const;


	/*!*********************************************************************************************************************
	\brief   Set the command line to a new string (wide).
	\param 	 cmdLine The new (wide) string to set the command line to
	***********************************************************************************************************************/
	void set(const wchar_t* cmdLine);

	/*!*********************************************************************************************************************
	\brief   Set the command line to a new list of arguments.
	\param 	 argc The number of arguments
	\param 	 argv The list of arguments
	***********************************************************************************************************************/
	void set(int argc, char** argv);

	/*!*********************************************************************************************************************
	\brief   Set the command line from a new string.
	\param 	 cmdLine The new string to set the command line to
	***********************************************************************************************************************/
	void set(const char* cmdLine);

	/*!*********************************************************************************************************************
	\brief   Set the command line from a stream.
	\param 	 stream The stream containing the new command line
	***********************************************************************************************************************/
	void set(Stream* stream);

	/*!*********************************************************************************************************************
	\brief   Set the command line from another command line.
	\param 	 commandLine Copy the data from another command line
	***********************************************************************************************************************/
	void set(const CommandLineParser& commandLine);

	/*!*********************************************************************************************************************
	\brief   Prepend data to the command line.
	\param 	 cmdLine A string containing the data to prepend to this command line
	***********************************************************************************************************************/
	void prefix(const wchar_t* cmdLine);

	/*!*********************************************************************************************************************
	\brief   Prepend a new list of arguments to the command line.
	\param 	 argc The number of arguments
	\param 	 argv The list of arguments
	***********************************************************************************************************************/
	void prefix(int argc, char** argv);

	/*!*********************************************************************************************************************
	\brief   Prepend data from a string to the command line.
	\param 	 cmdLine The string whose data to prepend to the command line
	***********************************************************************************************************************/
	void prefix(const char* cmdLine);

	/*!*********************************************************************************************************************
	\brief   Prepend data from a stream to the command line.
	\param 	 cmdLine The stream whose data to prepend to the command line
	***********************************************************************************************************************/
	void prefix(Stream* const cmdLine);

	/*!*********************************************************************************************************************
	\brief   Prepend the data from another command line.
	\param 	 cmdLine The command line from which to prepend the data
	***********************************************************************************************************************/
	void prefix(const CommandLineParser& cmdLine);

	/*!*********************************************************************************************************************
	\brief   Append data to the command line.
	\param 	 cmdLine A string containing the data to append to this command line
	***********************************************************************************************************************/
	void append(const wchar_t* cmdLine);

	/*!*********************************************************************************************************************
	\brief   Append a new list of arguments to the command line.
	\param 	 argc The number of arguments
	\param 	 argv The list of arguments
	***********************************************************************************************************************/
	void append(int argc, char** argv);

	/*!*********************************************************************************************************************
	\brief   Append data from a string to the command line.
	\param 	 cmdLine The string whose data to append to the command line
	***********************************************************************************************************************/
	void append(const char* cmdLine);

	/*!*********************************************************************************************************************
	\brief   Append data from a stream to the command line.
	\param 	 cmdLine The stream whose data to append to the command line
	***********************************************************************************************************************/
	void append(Stream* cmdLine);

	/*!*********************************************************************************************************************
	\brief   Append data from from another command line.
	\param 	  cmdLine The command line from which to append the data
	***********************************************************************************************************************/
	void append(const CommandLineParser& cmdLine);

protected:
	/*!*********************************************************************************************************************
	\brief    Parse an entire string for command line data.
	\param 	  cmdLine The string containing the command line data
	***********************************************************************************************************************/
	void parseCmdLine(const char8* const cmdLine);

	/*!*********************************************************************************************************************
	\brief    Parse a single argument as passed by the C/C++ style argc/argv command line format.
	\param 	  arg A single element of the "argv" array of char*
	***********************************************************************************************************************/
	void parseArgV(char* arg);

private:
	uint32 findArg(const char* pArg) const;
	bool readFlag(const char* pArg, bool& bVal) const;
	bool readUint(const char* pArg, unsigned int& uiVal) const;
	bool readFloat(const char* pArg, float& fVal) const;
	CharBuffer m_data;
	ParsedCommandLine m_commandLine;

};
typedef CommandLineParser::ParsedCommandLine CommandLine;
}

}