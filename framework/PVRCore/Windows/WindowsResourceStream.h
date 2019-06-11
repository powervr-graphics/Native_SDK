/*!
\brief A Stream implementation used to access Windows embedded resources.
\file PVRCore/Windows/WindowsResourceStream.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/stream/BufferStream.h"
#include <string>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>

namespace pvr {
/// <summary>A Stream implementation that is used to access resources built in a MS Windows executable.</summary>
/// <remarks>This Stream abstraction allows the user to easily access the Resources embedded in Microsoft Windows
/// .exe/.dll files created using Windows Resource Files. This is the default way resources are packed in the MS
/// Windows version of the PowerVR Examples.</remarks>
class WindowsResourceStream : public BufferStream
{
public:
	/// <summary>Constructor. Creates a new WindowsResourceStream from a Windows Embedded Resource.</summary>
	/// <param name="resourceName">The "filename". must be the same as the identifier of the Windows Embedded Resource</param>
	WindowsResourceStream(const std::string& resourceName) : BufferStream(resourceName)
	{
		_isReadable = true;

		// Find a handle to the resource
		HRSRC hR = FindResource(GetModuleHandle(NULL), resourceName.c_str(), RT_RCDATA);

		if (!hR)
		{
			throw FileNotFoundError(resourceName);
		}
		// Get a global handle to the resource data, which allows us to query the data pointer
		HGLOBAL hG = LoadResource(NULL, hR);

		if (!hG)
		{
			throw FileNotFoundError(resourceName);
		}
		// Get the data pointer itself. NB: Does not actually lock anything.
		_originalData = LockResource(hG);
		_bufferSize = SizeofResource(NULL, hR);
	}
};
} // namespace pvr
