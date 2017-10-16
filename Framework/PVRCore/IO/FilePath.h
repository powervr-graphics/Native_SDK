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
class FilePath : public string
{
public:
	/// <summary>Creates an empty Filepath object.</summary>
	FilePath();

	/// <summary>Creates an Filepath object from a filename.</summary>
	FilePath(const string& str);

	/// <summary>Get a string containing the Extension of the filepath.</summary>
	string getFileExtension() const;

	/// <summary>Get a string containing the Directory of the filepath.</summary>
	string getDirectory() const;

	/// <summary>Get a string containing the Filename+Extension of the filepath.</summary>
	string getFilename() const;

	/// <summary>Get a string containing the Filename (without extension) of the filepath.</summary>
	string getFilenameNoExtension() const;

	/// <summary>Get the directory separator used by the current platform.</summary>
    static char8 getDirectorySeparator();

private:
	static const char8 c_unixDirectorySeparator = '/';
	static const char8 c_windowsDirectorySeparator = '\\';
	static const char8 c_extensionSeparator = '.';
};
}
