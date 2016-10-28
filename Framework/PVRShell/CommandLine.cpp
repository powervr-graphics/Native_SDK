/*!*********************************************************************************************************************
\file         PVRShell\CommandLine.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Implementation for the CommandLine class.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRShell/CommandLine.h"

#include <cstring>
#include <cstdlib>
#if !defined(__APPLE__)
#include <malloc.h>
#endif

#if !defined(_WIN32)
#define _stricmp strcasecmp
#endif
using std::vector;
namespace pvr {
namespace platform {

CommandLineParser::CommandLineParser() : m_data(0)
{
}

// prefix, Set, Append methods
void CommandLineParser::prefix(const wchar* pwCmdLine)
{
	if (m_commandLine.m_options.size())
	{
		CommandLineParser tmp;
		tmp.set(pwCmdLine);
		prefix(tmp);
	}
	else
	{
		set(pwCmdLine);
	}

}

void CommandLineParser::prefix(int argc, char8** argv)
{
	if (m_commandLine.m_options.size())
	{
		CommandLineParser tmp;
		tmp.set(argc, argv);
		prefix(tmp);
	}
	else
	{
		set(argc, argv);
	}

}

void CommandLineParser::prefix(const char8* pCmdLine)
{
	if (m_commandLine.m_options.size())
	{
		CommandLineParser tmp;
		tmp.set(pCmdLine);
		prefix(tmp);
	}
	else
	{
		return set(pCmdLine);
	}
}

void CommandLineParser::prefix(Stream* const stream)
{
	if (m_commandLine.m_options.size())
	{
		CommandLineParser tmp;
		tmp.set(stream);
		prefix(tmp);
	}
	else
	{
		return set(stream);
	}
}

void CommandLineParser::prefix(const CommandLineParser& commandLine)
{
	if (commandLine.m_commandLine.m_options.size() == 0)
	{
		return;
	}

	CharBuffer newData; newData.resize(m_data.size() + commandLine.m_data.size());

	vector<ParsedCommandLine::Option> newOptions;
	newOptions.resize(m_commandLine.m_options.size() + commandLine.m_commandLine.m_options.size());

	// copy the data
	memcpy(newData.data(), commandLine.m_data.data(), commandLine.m_data.size());
	memcpy(&newData[commandLine.m_data.size()], m_data.data(), m_data.size());

	// Initialize the options
	for (unsigned int i = 0; i < commandLine.m_commandLine.m_options.size(); ++i)
	{
		newOptions[i].arg = (const char*)((size_t)commandLine.m_commandLine.m_options[i].arg - (size_t)commandLine.m_data.data()) +
		                    (size_t)newData.data();
		newOptions[i].val = (const char*)((size_t)commandLine.m_commandLine.m_options[i].val - (size_t)commandLine.m_data.data()) +
		                    (size_t)newData.data();
	}

	for (unsigned int i = 0; i < m_commandLine.m_options.size(); ++i)
	{
		newOptions[commandLine.m_commandLine.m_options.size() + i].arg = (const char*)((size_t)m_commandLine.m_options[i].arg - (size_t)m_data.data()) +
		    (size_t)newData.data() + commandLine.m_data.size();
		newOptions[commandLine.m_commandLine.m_options.size() + i].val = (const char*)((size_t)m_commandLine.m_options[i].val - (size_t)m_data.data()) +
		    (size_t)newData.data() + commandLine.m_data.size();
	}

	// Set the variables
	m_data = newData;

	m_commandLine.m_options = newOptions;
}

void CommandLineParser::set(const wchar_t* pwCmdLine)
{
	if (!pwCmdLine)
	{
		return;
	}

	size_t length = wcslen(pwCmdLine) + 1;

	std::vector<char8> tmp; tmp.resize(length);

	while (length)
	{
		--length;
		tmp[length] = static_cast<char8>(pwCmdLine[length]);
	}

	parseCmdLine(tmp.data());
}

void CommandLineParser::set(int argc, char** argv)
{
	if (argc < 0)
	{
		return;
	}

	m_commandLine.m_options.clear();

	{
		size_t length = 0;

		for (int i = 0; i < argc; ++i)
		{
			length += strlen(argv[i]) + 1;
		}

		m_data.resize(length);
	}

	size_t offset = 0;

	for (int i = 0; i < argc; ++i)
	{
		// length
		const size_t length = strlen(argv[i]) + 1;

		memcpy(&m_data[offset], argv[i], length);

		// split into var/arg
		parseArgV(&m_data[offset]);

		offset += length;
	}
}

void CommandLineParser::set(const char* pCmdLine)
{
	parseCmdLine(pCmdLine);
}

void CommandLineParser::set(Stream* const stream)
{
	if (!stream || !stream->isopen() || !stream->isReadable())
	{
		return;
	}
	size_t size = static_cast<size_t>(stream->getSize());

	if (size)
	{
		std::vector<byte> tmp; tmp.resize(size + 1);

		size_t dataRead;
		stream->read(1, size, tmp.data(), dataRead);

		size_t i = 0;

		for (; i < size; ++i)
		{
			switch (tmp[i])
			{
			case '\n':
			case '\r':
			case '\t':
				tmp[i] = ' ';
				break;
			default:
				break;
			}
		}

		tmp[i] = '\0';

		set((char8*)tmp.data());

	}
}

void CommandLineParser::set(const CommandLineParser& commandLine)
{
	*this = commandLine;;
}

void CommandLineParser::append(const wchar_t* pwCmdLine)
{
	if (m_commandLine.m_options.size())
	{
		CommandLineParser tmp;
		tmp.set(pwCmdLine);
		append(tmp);
	}
	else
	{
		set(pwCmdLine);
	}

}

void CommandLineParser::append(const int argc, char** argv)
{
	if (m_commandLine.m_options.size())
	{
		CommandLineParser tmp;
		tmp.set(argc, argv);
		append(tmp);
	}
	else
	{
		set(argc, argv);
	}

}

void CommandLineParser::append(const char* pCmdLine)
{
	if (m_commandLine.m_options.size())
	{
		CommandLineParser tmp;
		tmp.set(pCmdLine);
		append(tmp);
	}
	else
	{
		set(pCmdLine);
	}
}

void CommandLineParser::append(Stream* const stream)
{
	if (m_commandLine.m_options.size())
	{
		CommandLineParser tmp;
		tmp.set(stream);
		append(tmp);
	}
	else
	{
		set(stream);
	}
}

void CommandLineParser::append(const CommandLineParser& commandLine)
{
	if (commandLine.m_commandLine.m_options.size() == 0)
	{
		return;
	}

	CharBuffer newData; newData.resize(m_data.size() + commandLine.m_data.size());

	vector<ParsedCommandLine::Option> newOptions;
	newOptions.resize(m_commandLine.m_options.size() + commandLine.m_commandLine.m_options.size());

	// copy the data
	memcpy(newData.data(), m_data.data(), m_data.size());
	memcpy(&newData[m_data.size()], commandLine.m_data.data(), commandLine.m_data.size());

	// Initialize the options
	for (unsigned int i = 0; i < m_commandLine.m_options.size(); ++i)
	{
		newOptions[i].arg = (const char*)((size_t)m_commandLine.m_options[i].arg - (size_t)m_data.data()) + (size_t)newData.data();
		newOptions[i].val = (const char*)((size_t)m_commandLine.m_options[i].val - (size_t)m_data.data()) + (size_t)newData.data();
	}

	for (unsigned int i = 0; i < commandLine.m_commandLine.m_options.size(); ++i)
	{
		newOptions[m_commandLine.m_options.size() + i].arg = (const char*)((size_t)commandLine.m_commandLine.m_options[i].arg - (size_t)commandLine.m_data.data()) +
		    (size_t)newData.data() + m_data.size();
		newOptions[m_commandLine.m_options.size() + i].val = (const char*)((size_t)commandLine.m_commandLine.m_options[i].val - (size_t)commandLine.m_data.data()) +
		    (size_t)newData.data() + m_data.size();
	}

	// Set the variables
	m_data = newData;
	m_commandLine.m_options = newOptions;
}

void CommandLineParser::parseCmdLine(const char8* const commandLine)
{
	size_t		len;
	int			nIn, nOut;
	bool		bInQuotes;
	ParsedCommandLine::Option	opt;

	if (!commandLine)
	{
		return;
	}

	// Take a copy of the original
	len = strlen(commandLine) + 1;

	// Take a copy to be edited
	m_data.resize(len);

	// Break the command line into options
	bInQuotes = false;
	opt.arg = NULL;
	opt.val = NULL;
	nIn = -1;
	nOut = 0;
	do
	{
		++nIn;
		if (commandLine[nIn] == '"')
		{
			bInQuotes = !bInQuotes;
		}
		else
		{
			if (bInQuotes && commandLine[nIn] != 0)
			{
				if (!opt.arg)
				{
					opt.arg = &m_data[nOut];
				}

				m_data[nOut++] = commandLine[nIn];
			}
			else
			{
				switch (commandLine[nIn])
				{
				case '=':
					m_data[nOut++] = 0;
					opt.val = &m_data[nOut];
					break;

				case ' ':
				case '\t':
				case '\0':
					m_data[nOut++] = 0;
					if (opt.arg || opt.val)
					{
						// Add option to list
						m_commandLine.m_options.push_back(opt);

						opt.arg = NULL;
						opt.val = NULL;
					}
					break;

				default:
					if (!opt.arg)
					{
						opt.arg = &m_data[nOut];
					}

					m_data[nOut++] = commandLine[nIn];
					break;
				}
			}
		}
	}
	while (commandLine[nIn]);
}

void CommandLineParser::parseArgV(char* const pStr)
{
	ParsedCommandLine::Option	opt;
	size_t				j;

	// Hunt for an = symbol
	for (j = 0; pStr[j] && pStr[j] != '='; ++j);

	opt.arg = pStr;
	if (pStr[j])
	{
		// terminate the arg string, set value string
		pStr[j] = 0;
		opt.val = &pStr[j + 1];
	}
	else
	{
		// no value specified
		opt.val = 0;
	}

	// Add option to list
	m_commandLine.m_options.push_back(opt);
}

const CommandLineParser::ParsedCommandLine& CommandLineParser::getParsedCommandLine() const
{
	return m_commandLine;
}

uint32 CommandLineParser::findArg(const char* arg) const
{
	uint32 i;

	/*
		Find an argument, case insensitive. Returns the index of the option
		if found, or the number of options if not.
	*/
	for (i = 0; i < m_commandLine.m_options.size(); ++i)
	{
		if (_stricmp(m_commandLine.m_options[i].arg, arg) == 0)
		{
			break;
		}
	}

	return i;
}

bool CommandLineParser::readFlag(const char* arg, bool& bVal) const
{
	uint32 nIdx = findArg(arg);

	if (nIdx == m_commandLine.m_options.size())
	{
		return false;
	}
	else
	{
		// a flag must have no value
		bVal = m_commandLine.m_options[nIdx].val ? false : true;
		return true;
	}
}

bool CommandLineParser::readUint(const char* arg, unsigned int& uiVal) const
{
	uint32 nIdx = findArg(arg);

	if (nIdx == m_commandLine.m_options.size())
	{
		return false;
	}
	else
	{
		uiVal = atoi(m_commandLine.m_options[nIdx].val);
		return true;
	}
}

bool CommandLineParser::readFloat(const char* arg, float& fVal) const
{
	uint32 nIdx = findArg(arg);

	if (nIdx == m_commandLine.m_options.size())
	{
		return false;
	}
	else
	{
		fVal = static_cast<float>(atof(m_commandLine.m_options[nIdx].val));
		return true;
	}
}

bool CommandLineParser::ParsedCommandLine::hasOption(const char* name) const
{
	return std::find(m_options.begin(), m_options.end(), name) != m_options.end();
}
bool CommandLineParser::ParsedCommandLine::getStringOption(const char* name, std::string& outValue) const
{
	auto it = std::find(m_options.begin(), m_options.end(), name);
	if (it == m_options.end()) { return false; }
	outValue = it->val;
	return true;
}

bool CommandLineParser::ParsedCommandLine::getFloatOption(const char* name, float32& outValue) const
{
	auto it = std::find(m_options.begin(), m_options.end(), name);
	if (it == m_options.end()) { return false; }
	outValue = (float32)atof(it->val);
	return true;
}
bool CommandLineParser::ParsedCommandLine::getIntOption(const char* name, int32& outValue) const
{
	auto it = std::find(m_options.begin(), m_options.end(), name);
	if (it == m_options.end()) { return false; }
	outValue = atoi(it->val);
	return true;
}
bool CommandLineParser::ParsedCommandLine::getBoolOptionSetTrueIfPresent(const char* name, bool& outValue) const
{
	auto it = std::find(m_options.begin(), m_options.end(), name);
	if (it == m_options.end()) { return false; }
	outValue = true;
	return true;
}
bool CommandLineParser::ParsedCommandLine::getBoolOptionSetFalseIfPresent(const char* name, bool& outValue) const
{
	auto it = std::find(m_options.begin(), m_options.end(), name);
	if (it == m_options.end()) { return false; }
	outValue = false;
	return true;
}

const CommandLineParser::ParsedCommandLine::Options& CommandLineParser::ParsedCommandLine::getOptionsList() const { return m_options; }
}
}
//!\endcond