/*!
\brief Implementations of functions from the strings:: namespace.
\file PVRCore/Strings/StringFunctions.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "PVRCore/StringFunctions.h"

namespace pvr {
namespace strings {
void ignoreWhitespace(char8** pszString)
{
	while (*pszString[0] == '\t' ||
	       *pszString[0] == '\n' ||
	       *pszString[0] == '\r' ||
	       *pszString[0] == ' ')
	{
		(*pszString)++;
	}
}

char8* readEOLToken(char8* pToken)
{
	char* pReturn = NULL;

	char szDelim[2] = { '\n', 0 };				// try newline
	pReturn = strtok(pToken, szDelim);
	if (pReturn == NULL)
	{
		szDelim[0] = '\r';
		pReturn = strtok(pToken, szDelim);		// try linefeed
	}
	return pReturn;
}

bool concatenateLinesUntil(string& Out, int& nLine, const std::vector<string>& ppszLines, unsigned int nLimit,
                           const char* pszEnd)
{
	unsigned int	i, j;
	size_t			nLen;

	nLen = 0;
	for (i = nLine; i < nLimit; ++i)
	{
		if (strcmp(ppszLines[i].c_str(), pszEnd) == 0)
		{
			break;
		}
		nLen += strlen(ppszLines[i].c_str()) + 1;
	}
	if (i == nLimit)
	{
		return false;
	}

	if (nLen)
	{
		++nLen;

		Out.reserve(nLen);

		for (j = nLine; j < i; ++j)
		{
			Out.append(ppszLines[j]);
			Out.append("\n");
		}
	}

	nLine = i;
	return true;
}


}
}
//!\endcond