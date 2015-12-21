/*!*********************************************************************************************************************
\file         PVRCore\Windows\WindowsResourceStream.cpp
\author       PowerVR by Imagination, Developer Technology Team
\copyright    Copyright (c) Imagination Technologies Limited.
\brief         Implementations file for the WindowsResourceStream.
***********************************************************************************************************************/
//!\cond NO_DOXYGEN
#include "PVRCore/Windows/WindowsResourceStream.h"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
using std::string;
namespace pvr{
WindowsResourceStream::WindowsResourceStream(const string& fileName) : BufferStream(fileName)
{
	m_isReadable = true;

	// Find a handle to the resource
	HRSRC hR = FindResource(GetModuleHandle(NULL), fileName.c_str(), RT_RCDATA);

	if(hR)
	{
		// Get a global handle to the resource data, which allows us to query the data pointer
		HGLOBAL hG = LoadResource(NULL, hR);

		if(hG)
		{
			//Get the data pointer itself. NB: Does not actually lock anything.
			m_originalData = LockResource(hG);
			m_bufferSize = SizeofResource(NULL, hR);
		}
	}
}
}
//!\endcond