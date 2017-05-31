/*!
\brief Contains helper functions for string manipulation.
\file PVRCore/StringFunctions.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/CoreIncludes.h"
#include "PVRCore/Strings/UnicodeConverter.h"

namespace pvr {
/// <summary>Contains several valuable helpers to assist with common string operations: Starts with, ends with, create
/// with printf-style formatting and others.</summary>
namespace strings {
/// <summary>Creates an std::string from a printf style vararg list.</summary>
/// <param name="format">printf-style format string</param>
/// <param name="argumentList">printf-style va_list</param>
inline string vaFormatString(const char* format, va_list argumentList)
{
	using namespace std;
	Result result = Result::Success;

#if defined(_WIN32)
	int newStringSize = _vscprintf(format, argumentList);
#else
	va_list argumentListCopy;

	va_copy(argumentListCopy, argumentList);
	int newStringSize = vsnprintf(0, 0, format, argumentListCopy);
	va_end(argumentListCopy);
#endif

	// Create a new string
	string newString;
	newString.resize(static_cast<uint32>(newStringSize + 1));

	if (result == Result::Success)
	{
#ifdef _WIN32
		if (_vsnprintf_s(const_cast<char*>(newString.c_str()), newStringSize + 1, newStringSize, format, argumentList) != newStringSize)
#else
		pvr::int32 n = vsnprintf(const_cast<char*>(newString.data()), newStringSize + 1, format, argumentList);
		if ((n < 0) || (n >= newStringSize + 1))
#endif
		{
			result = Result::UnknownError;
		}
	}

	if (result != Result::Success)
	{
		newString.resize(0);
	}

	return string(newString);
}

/// <summary>Creates an std::wstring from a printf style vararg list.</summary>
/// <param name="format">printf-style wide format string</param>
/// <param name="argumentList">printf-style va_list</param>
inline std::basic_string<wchar> vaFormatString(const wchar* const format, va_list argumentList)
{
	Result result = Result::Success;

#if defined(_WIN32)
	int newStringSize = _vscwprintf(format, argumentList);
#else
	va_list argumentListCopy;

	va_copy(argumentListCopy, argumentList);
	int newStringSize = vswprintf(0, 0, format, argumentListCopy);
	va_end(argumentListCopy);
#endif

	// Create a new string
	std::basic_string<wchar> newString;
	newString.resize(static_cast<uint32>(newStringSize + 1));

	if (result == Result::Success)
	{

#ifdef _WIN32
		if (_vsnwprintf_s(const_cast<wchar*>(newString.c_str()), newStringSize + 1, newStringSize, format,
		                  argumentList) != newStringSize)
#else
		if (vswprintf((wchar_t*)newString.c_str(), (size_t)newStringSize + 1, (const wchar_t*)format, argumentList) != newStringSize)
#endif
		{
			result = Result::UnknownError;
		}
	}

	if (result != Result::Success)
	{
		newString.resize(0);
	}

	return std::basic_string<wchar>(newString);
}

/// <summary>Creates an std::string with a printf-like command.</summary>
/// <param name="format">printf-style format string</param>
/// <param name="...">printf-style variable arguments</param>
inline std::string createFormatted(const char8* const format, ...)
{
	// Calculate the length of the new string
	va_list argumentList;
	va_start(argumentList, format);
	string newString = strings::vaFormatString(format, argumentList);
	va_end(argumentList);

	return newString;
}

/// <summary>Creates an std::wstring with a printf-like command.</summary>
/// <param name="format">printf-style wide format string</param>
/// <param name="...">printf-style variable arguments</param>
inline std::basic_string<wchar> createFormatted(const wchar* const format, ...)
{
	// Calculate the length of the new string
	va_list argumentList;
	va_start(argumentList, format);
	std::basic_string<wchar> newString = strings::vaFormatString(format, argumentList);
	va_end(argumentList);
	return newString;
}

/// <summary>Transforms a string to lowercase in place.</summary>
/// <param name="str">A string to transform to lowercase.</param>
inline string& toLower(string& str)
{
	std::transform(str.begin(), str.end(), str.begin(), tolower);
	return str;
}

/// <summary>Transforms a string to lowercase in place.</summary>
/// <param name="str">A string to transform to lowercase.</param>
inline string toLower(const string& str)
{
	std::string s = str;
	return toLower(s);
}

/// <summary>Skips any beginning space, tab or new-line characters, advancing the pointer to the first
/// non-whitespace character</summary>
/// <param name="myString">Pointer to a c-style string. Will be advanced to the first non-whitespace char (or the null
/// terminator if no other characters exist)</param>
void ignoreWhitespace(char8** myString);

/// <summary>Reads next strings to the end of the line and interperts as a token.</summary>
/// <param name="pToken">The string</param>
/// <returns>char* The</returns>
char8* readEOLToken(char8* pToken);

/// <summary>Outputs a block of text starting from nLine and ending when the string endStr is found.</summary>
/// <param name="outStr">output text</param>
/// <param name="line">Input start line number, outputs end line number</param>
/// <param name="lines">Input text - one array element per line</param>
/// <param name="endStr">End string: When this string is encountered, the procedure will stop.</param>
/// <param name="limit">A limit to the number of lines concatenated</param>
/// <returns>true if successful, false if endStr was not found before lines finished or limit was reached
/// </returns>
bool concatenateLinesUntil(string& outStr, int& line, const std::vector<string>& lines, unsigned int limit,
                           const char* endStr);

/// <summary>Tests if a string starts with another string.</summary>
/// <param name="str">The string whose beginning will be checked</param>
/// <param name="substr">The sequence of characters to check if str starts with</param>
/// <returns>true if the string substr is indeed the first characters of str, false otherwise.</returns>
inline bool startsWith(const char* str, const char* substr)
{
	int current = 0;
	while (str[current] && substr[current])
	{
		if (str[current] != substr[current]) { return false; }
		++current;
	}
	if (!str[current] && substr[current]) { return false; }
	return true;
}
/// <summary>Tests if a string starts with another string.</summary>
/// <param name="str">The string whose beginning will be checked</param>
/// <param name="substr">The sequence of characters to check if str starts with</param>
/// <returns>true if the string substr is indeed the first characters of str, false otherwise.</returns>
inline bool startsWith(const std::string& str, const std::string& substr)
{
	return startsWith(str.c_str(), substr.c_str());
}

/// <summary>Tests if a string starts with another string.</summary>
/// <param name="str">The string whose beginning will be checked</param>
/// <param name="substr">The sequence of characters to check if str starts with</param>
/// <returns>true if the string substr is indeed the first characters of str, false otherwise.</returns>
inline bool startsWith(const std::string& str, const char* substr)
{
	return startsWith(str.c_str(), substr);
}

/// <summary>Tests if a string ends with another string.</summary>
/// <param name="str">The string whose end will be checked. Not null terminated - length is passed explicitly.
/// </param>
/// <param name="lenStr">The length of str</param>
/// <param name="substr">The sequence of characters to check if str ends with. Not null terminated - length is
/// passed explicitly.</param>
/// <param name="lenSubstr">The length of Substr</param>
/// <returns>true if the string substr is indeed the last characters of str, false otherwise.</returns>
inline bool endsWith(const char* str, int32 lenStr, const char* substr, int32 lenSubstr)
{
	if (lenSubstr > lenStr || !lenStr--) { return false; }
	if (!lenSubstr--) { return true; }
	while (lenStr >= 0 && lenSubstr >= 0)
	{
		if (str[lenStr--] != substr[lenSubstr--]) { return false; }
	}
	if (!lenStr && lenSubstr) { return false; }
	return true;
}

/// <summary>Tests if a string ends with another string.</summary>
/// <param name="str">The string whose end will be checked.</param>
/// <param name="substr">The sequence of characters to check if str ends with.</param>
/// <returns>true if the string substr is indeed the last characters of str, false otherwise.</returns>
inline bool endsWith(const std::string& str, const std::string& substr)
{
	return endsWith(str.c_str(), (int32)str.length(), substr.c_str(), (int32)substr.length());
}

/// <summary>Tests if a string starts with another string.</summary>
/// <param name="str">The string whose end will be checked.</param>
/// <param name="substr">The sequence of characters to check if str starts with.</param>
/// <returns>true if the string substr is indeed the first characters of str, false otherwise.</returns>
inline bool endsWith(const std::string& str, const char* substr)
{
	return endsWith(str.c_str(), (int32)str.length(), substr, (int32)strlen(substr));
}

/// <summary>Tests if a string starts with another string.</summary>
/// <param name="str">The string whose end will be checked.</param>
/// <param name="substr">The sequence of characters to check if str starts with.</param>
/// <returns>true if the string substr is indeed the first characters of str, false otherwise.</returns>
inline bool endsWith(const char* str, const char* substr)
{
	return endsWith(str, (int32)strlen(str), substr, (int32)strlen(substr));
}

/// <summary>Separate a filename to name and extension</summary>
/// <param name="fileAndExtension">A filename</param>
/// <param name="filename">The file part of the name (part before the last '.')</param>
/// <param name="extension">The extension part of the name (part after the last '.')</param>
/// <remarks>The period is returned in neither filename nor the extension</remarks>
inline void getFileNameAndExtension(const std::string& fileAndExtension, std::string& filename, std::string& extension)
{
	auto it = std::find(fileAndExtension.rbegin(), fileAndExtension.rend(), '.');
	if (it == fileAndExtension.rend())
	{
		filename = fileAndExtension;
		extension = "";
		return;
	}
	size_t position = fileAndExtension.rend() - it; //rend is the position one-after the start of the string.
	filename.assign(fileAndExtension.begin(), fileAndExtension.begin() + position - 1);
	extension.assign(fileAndExtension.begin() + position, fileAndExtension.end());
}
}
}