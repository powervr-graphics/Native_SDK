/*!
\brief Implementation of the FileWrapStream class methods.
\file PVRCore/IO/FileWrapStream.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "PVRCore/IO/FileWrapStream.h"
pvr::FileWrapStream::FileWrapStream(const std::string& fileName) : BufferStream(fileName)
{
	_isReadable = true;
	std::map<std::string, SizedPointer<void> >::iterator found = getFileRegistry().find(fileName);

	if (found != FileWrapStream::getFileRegistry().end())
	{
		_originalData = found->second;
		_bufferSize = found->second.getSize();
	}
	else
	{
		_originalData = NULL;
	}
}
//!\endcond