/*!
\brief Implementation of the FilePath class
\file PVRCore/IO/FilePath.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "PVRCore/IO/FilePath.h"
using std::basic_string;

namespace pvr {

FilePath::FilePath() : std::string()
{
}

FilePath::FilePath(const std::string& str) : std::string(str)
{
}

std::string FilePath::getFileExtension() const
{
	size_t index = find_last_of(c_extensionSeparator, length());

	if (index != std::string::npos)
	{
		return substr((index + 1), length() - (index + 1));
	}

	return std::string();
}

std::string FilePath::getDirectory() const
{
	const std::string::size_type c_objectNotFound = std::numeric_limits<std::string::size_type>::max();

	std::string::size_type index = static_cast<std::string::size_type>(find_last_of(c_unixDirectorySeparator, length()));

#if defined(_WIN32)
	if (index == std::string::npos)
	{
		index = static_cast<std::string::size_type>(find_last_of(c_windowsDirectorySeparator, length()));
	}
#endif

	if (index != c_objectNotFound)
	{
		return substr(0, index);
	}

	return std::string();
}

std::string FilePath::getFilename() const
{
	std::string::size_type  index = static_cast<std::string::size_type>(find_last_of(c_unixDirectorySeparator, length()));

#if defined(_WIN32)
	if (index == std::string::npos)
	{
		index = static_cast<std::string::size_type>(find_last_of(c_windowsDirectorySeparator, length()));
	}
#endif

	if (index != std::string::npos)
	{
		return substr((index + 1), length() - (index + 1));
	}

	return *this;
}

std::string FilePath::getFilenameNoExtension() const
{
	std::string::size_type extensionIndex = static_cast<std::string::size_type>(find_last_of(c_extensionSeparator, length()));

	if (extensionIndex != std::string::npos)
	{
		return getFilename().substr(0, extensionIndex);
	}

	return getFilename();
}

char FilePath::getDirectorySeparator()
{
#if defined(WIN32) || defined(_WIN32)
	return c_windowsDirectorySeparator;
#else
	return c_unixDirectorySeparator;
#endif
}
}
//!\endcond