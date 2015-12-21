/*!*********************************************************************************************************************
\file         PVRCore\FilePath.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief		  Implementation of the FilePath class
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRCore/FilePath.h"
using std::basic_string;

namespace pvr
{

FilePath::FilePath() : string() 
{
}

FilePath::FilePath(const string& str) : string(str)
{
}

string FilePath::getFileExtension() const
{
	size_t index = find_last_of(c_extensionSeparator, length());

	if (index != string::npos)
	{
		return substr( (index + 1), length() - (index + 1) );
	}

	return string();
}

string FilePath::getDirectory() const
{
	const uint32 c_objectNotFound = 0xffffffffu;

	uint32 index = (uint32)find_last_of(c_unixDirectorySeparator, length());

#if defined(_WIN32)
	if(index == string::npos)
	{
		index = (uint32)find_last_of(c_windowsDirectorySeparator, length());
	}
#endif

	if(index != c_objectNotFound)
	{
		return substr(0,index);
	}
	
	return string();
}

string FilePath::getFilename() const
{
	string::size_type  index = (string::size_type )find_last_of(c_unixDirectorySeparator, length());

#if defined(_WIN32)
	if (index == string::npos)
	{
		index = (uint32)find_last_of(c_windowsDirectorySeparator, length());
	}
#endif
    
	if(index != string::npos)
	{
		return substr( (index+1), length() - (index+1) );
	}

	return *this;
}

string FilePath::getFilenameNoExtension() const
{
	string::size_type extensionIndex = (string::size_type )find_last_of(c_extensionSeparator, length());

	if(extensionIndex != string::npos)
	{
		return getFilename().substr(0,extensionIndex);
	}

	return getFilename();
}

const tchar FilePath::getDirectorySeparator()
{
#if defined(WIN32) || defined(_WIN32)
	return c_windowsDirectorySeparator;
#else
	return c_unixDirectorySeparator;
#endif
}
}
//!\endcond