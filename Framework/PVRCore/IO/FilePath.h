/*!
\brief Provides a class representing a Filepath.
\file PVRCore/IO/FilePath.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRCore/Base/Defines.h"
#pragma once

namespace pvr {
/// <summary>Filepath represents a Path + Filename + Extension.</summary>
class FilePath : public std::string
{
public:
	/// <summary>Creates an empty Filepath object.</summary>
	FilePath();

	/// <summary>Creates an Filepath object from a filename.</summary>
	/// <param name="str">Creates a filepath object of this path.</param>
	FilePath(const std::string& str);

	/// <summary>Get a std::string containing the Extension of the filepath.</summary>
	/// <returns>The file extension of the path</returns>
	std::string getFileExtension() const;

	/// <summary>Get a std::string containing the Directory of the filepath.</summary>
	/// <returns>The directory part of the path</returns>
	std::string getDirectory() const;

	/// <summary>Get a std::string containing the Filename+Extension of the filepath.</summary>
	/// <returns>The filename part of the path (including the extension)</returns>
	std::string getFilename() const;

	/// <summary>Get a std::string containing the Filename (without extension) of the filepath.</summary>
	/// <returns>The filename part of the path (without the extension)</returns>
	std::string getFilenameNoExtension() const;

	/// <summary>Get the directory separator used by the current platform.</summary>
	/// <returns>The platform specific directory separator (usually "\\" or "/"</returns>
	static char getDirectorySeparator();

private:
	static const char c_unixDirectorySeparator = '/';
	static const char c_windowsDirectorySeparator = '\\';
	static const char c_extensionSeparator = '.';
};
}
